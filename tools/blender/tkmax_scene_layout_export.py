"""TKMAX scene layout exporter. Install this single .py as a Blender 4.x legacy add-on.
Pure coordinate/OBJ helpers also run without Blender for automated validation.
"""
import hashlib
import json
import math
import os
from pathlib import Path
import uuid

bl_info = {
    "name": "TKMAX Scene Layout Export", "author": "TKMAX", "version": (1, 0, 0),
    "blender": (4, 0, 0), "location": "File > Export / Scene Properties",
    "description": "Export scene layout placement, evaluated OBJ meshes and basic diffuse textures", "category": "Import-Export",
}


def multiply(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(4)) for j in range(4)] for i in range(4)]


def convert_point(p):
    # Blender: Z up, +Y forward -> TKMAX: Y up, +Z forward (left handed).
    return (float(p[0]), float(p[2]), float(p[1]))


def compose_transform(position, rotation, scale):
    x, y, z = rotation
    cx, sx, cy, sy, cz, sz = math.cos(x), math.sin(x), math.cos(y), math.sin(y), math.cos(z), math.sin(z)
    r = [[cy*cz, cz*sy*sx-sz*cx, cz*sy*cx+sz*sx],
         [cy*sz, sz*sy*sx+cz*cx, sz*sy*cx-cz*sx], [-sy, cy*sx, cy*cx]]
    return [[r[i][j]*scale[j] if j < 3 else position[i] for j in range(4)] for i in range(3)] + [[0, 0, 0, 1]]


def engine_transform(blender_world):
    c = [[1, 0, 0, 0], [0, 0, 1, 0], [0, 1, 0, 0], [0, 0, 0, 1]]
    m = multiply(multiply(c, blender_world), c)
    if any(not math.isfinite(v) or abs(v) > 1_000_000 for row in m for v in row):
        raise ValueError("Transform contains invalid or excessively large values")
    scale = [math.sqrt(sum(m[i][j]**2 for i in range(3))) for j in range(3)]
    if min(scale) < 0.0001:
        raise ValueError("Zero scale is not supported")
    determinant = sum(m[0][j]*(m[1][(j+1)%3]*m[2][(j+2)%3]-m[1][(j+2)%3]*m[2][(j+1)%3]) for j in range(3))
    if determinant < 0:
        scale[0] = -scale[0]
    r = [[m[i][j]/scale[j] for j in range(3)] for i in range(3)]
    cy = math.hypot(r[0][0], r[1][0])
    rotation = [math.atan2(r[2][1], r[2][2]) if cy > 1e-7 else math.atan2(-r[1][2], r[1][1]),
                math.atan2(-r[2][0], cy), math.atan2(r[1][0], r[0][0]) if cy > 1e-7 else 0.0]
    position = [m[i][3] for i in range(3)]
    rebuilt = compose_transform(position, rotation, scale)
    if any(abs(m[i][j]-rebuilt[i][j]) > 1e-4*max(1.0, abs(m[i][j])) for i in range(4) for j in range(4)):
        raise ValueError("Parenting produced shear; apply scale/transform before exporting")
    return {"position": position, "rotation": rotation, "scale": scale}


def obj_body(triangles):
    """triangles: [(material index, [(position, uv, normal) * 3])]."""
    lines, vertex_count, previous_material = [], 0, None
    for material, corners in triangles:
        if material != previous_material:
            lines.append(f"usemtl m{material}")
            previous_material = material
        indices = []
        for position, uv, normal in corners:
            p, n = convert_point(position), convert_point(normal)
            # Current TKMAX loader flips normal.x and face order. Compensate here,
            # without changing existing model import behavior.
            lines.append("v %.9g %.9g %.9g" % p)
            lines.append("vt %.9g %.9g" % (uv[0], uv[1]))
            lines.append("vn %.9g %.9g %.9g" % (-n[0], n[1], n[2]))
            vertex_count += 1
            indices.append(f"{vertex_count}/{vertex_count}/{vertex_count}")
        lines.append("f " + " ".join(indices))
    return "\n".join(lines) + "\n"


def atomic_write(path, data):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    try:
        temporary.write_bytes(data)
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


try:
    import bpy
    from bpy_extras.io_utils import ExportHelper
except ImportError:
    bpy = None


if bpy is not None:
    def export_texture(material, directory, scene):
        image, color = None, (0.8, 0.8, 0.8, 1.0)
        if material is not None:
            color = tuple(material.diffuse_color)
            if material.use_nodes:
                principled = next((n for n in material.node_tree.nodes if n.type == 'BSDF_PRINCIPLED'), None)
                if principled:
                    base = principled.inputs.get('Base Color')
                    color = tuple(base.default_value)
                    if base.is_linked:
                        node = base.links[0].from_node
                        if node.type != 'TEX_IMAGE' or node.image is None:
                            raise ValueError(f"{material.name}: bake procedural Base Color to an image first")
                        image = node.image
        if image and image.source not in {'FILE', 'GENERATED'}:
            raise ValueError(f"{image.name}: tiled/movie/sequence textures are not supported")
        directory.mkdir(parents=True, exist_ok=True)
        temporary = directory / ("temporary_" + uuid.uuid4().hex + ".png")
        copy = image.copy() if image else bpy.data.images.new("TKMAX_export_color", width=1, height=1, alpha=True)
        try:
            if image is None:
                copy.pixels = color
            copy.filepath_raw, copy.file_format = str(temporary), 'PNG'
            copy.save()
            data = temporary.read_bytes()
        finally:
            bpy.data.images.remove(copy)
            if temporary.exists():
                temporary.unlink()
        name = hashlib.sha256(data).hexdigest()[:24] + ".png"
        atomic_write(directory / name, data)
        return "textures/" + name

    def export_mesh(obj, depsgraph, assets, scene, texture_cache):
        evaluated = obj.evaluated_get(depsgraph)
        mesh = evaluated.to_mesh(preserve_all_data_layers=True, depsgraph=depsgraph)
        try:
            mesh.calc_loop_triangles()
            if hasattr(mesh, 'calc_normals_split'):
                mesh.calc_normals_split()
            if not mesh.loop_triangles:
                raise ValueError(f"{obj.name}: mesh has no triangles")
            uv_layer = mesh.uv_layers.active
            triangles, used_materials = [], set()
            for triangle in mesh.loop_triangles:
                material_index = triangle.material_index
                used_materials.add(material_index)
                corners = []
                for loop_index in triangle.loops:
                    loop = mesh.loops[loop_index]
                    normal = mesh.corner_normals[loop_index].vector if hasattr(mesh, 'corner_normals') else loop.normal
                    uv = tuple(uv_layer.data[loop_index].uv) if uv_layer else (0.0, 0.0)
                    corners.append((tuple(mesh.vertices[loop.vertex_index].co), uv, tuple(normal)))
                triangles.append((material_index, corners))
            # The current OBJ loader expects each texture's faces to be contiguous.
            # Merge identical textures and sort the triangles into contiguous groups.
            textures = {}
            for index in sorted(used_materials):
                material = mesh.materials[index] if index < len(mesh.materials) else None
                key = material.as_pointer() if material else None
                if key not in texture_cache:
                    texture_cache[key] = export_texture(material, assets / "textures", scene)
                textures[index] = texture_cache[key]
            unique = sorted(set(textures.values()))
            canonical = {index: unique.index(texture) for index, texture in textures.items()}
            triangles = sorted([(canonical[index], corners) for index, corners in triangles], key=lambda item: item[0])
            material_lines = []
            for index, texture in enumerate(unique):
                material_lines.extend([f"newmtl m{index}", "map_Kd " + texture])
            body, mtl = obj_body(triangles), "\n".join(material_lines) + "\n"
            # Identical evaluated meshes/materials share an asset. Changed geometry
            # gets a new path, so TKMAX's existing cache can load the new version.
            key = hashlib.sha256((body + mtl).encode('utf-8')).hexdigest()[:24]
            atomic_write(assets / (key + ".mtl"), mtl.encode('utf-8'))
            atomic_write(assets / (key + ".obj"), ("mtllib " + key + ".mtl\n" + body).encode('utf-8'))
            return "blender_layout_assets/" + key + ".obj"
        finally:
            evaluated.to_mesh_clear()

    def export_scene_layout(context, filepath, selected_only):
        output = Path(filepath).expanduser().resolve()
        if output.parent.name != "layouts" or output.parent.parent.name != "resources":
            raise ValueError("Choose project/resources/layouts/<name>.json as the export path")
        project = output.parent.parent.parent
        if not (project / "resources" / "obj").is_dir():
            raise ValueError("Selected directory is not a TKMAX project (resources/obj is missing)")
        objects, markers, texture_cache = [], [], {}
        depsgraph = context.evaluated_depsgraph_get()
        source = context.selected_objects if selected_only else context.scene.objects
        for obj in sorted(source, key=lambda o: o.name_full):
            if obj.hide_render or not obj.visible_get(view_layer=context.view_layer) or obj.get("tkmax_exclude", False):
                continue
            if obj.instance_type == 'COLLECTION':
                raise ValueError(f"{obj.name}: make collection instances real before export")
            if obj.type not in {'MESH', 'EMPTY'}:
                continue
            try:
                transform = engine_transform([[float(v) for v in row] for row in obj.matrix_world])
                if obj.type == 'EMPTY':
                    markers.append({"name": obj.name_full, "tag": str(obj.get("tkmax_tag", "")), **transform})
                else:
                    model = export_mesh(obj, depsgraph, project / "resources/obj/blender_layout_assets", context.scene, texture_cache)
                    objects.append({"name": obj.name_full, "model": model, "visible": True, **transform})
            except Exception as exc:
                raise ValueError(f"{obj.name}: {exc}") from exc
        if len(objects) > 4096 or len(markers) > 4096:
            raise ValueError("SceneLayout supports up to 4096 meshes and 4096 markers")
        data = {"format": "TKMAX_SCENE_LAYOUT", "version": 1, "objects": objects, "markers": markers}
        atomic_write(output, json.dumps(data, ensure_ascii=False, indent=2, allow_nan=False).encode('utf-8'))
        return len(objects), len(markers)

    class TKMAX_OT_export_scene_layout(bpy.types.Operator, ExportHelper):
        bl_idname = "export_scene.tkmax_scene_layout"
        bl_label = "TKMAX Scene Layout (.json)"
        filename_ext = ".json"
        filter_glob: bpy.props.StringProperty(default="*.json", options={'HIDDEN'})
        selected_only: bpy.props.BoolProperty(name="Selected objects only", default=False)

        def execute(self, context):
            try:
                count, markers = export_scene_layout(context, self.filepath, self.selected_only)
                self.report({'INFO'}, f"Exported {count} meshes and {markers} markers")
                return {'FINISHED'}
            except Exception as exc:
                self.report({'ERROR'}, str(exc))
                return {'CANCELLED'}

    class TKMAX_PT_scene_layout(bpy.types.Panel):
        bl_label = "TKMAX Scene Layout"
        bl_idname = "SCENE_PT_tkmax_scene_layout"
        bl_space_type = 'PROPERTIES'
        bl_region_type = 'WINDOW'
        bl_context = "scene"

        def draw(self, context):
            self.layout.label(text="Export to project/resources/layouts/*.json")
            self.layout.operator(TKMAX_OT_export_scene_layout.bl_idname)

    def menu_export(self, context):
        self.layout.operator(TKMAX_OT_export_scene_layout.bl_idname)

    def register():
        bpy.utils.register_class(TKMAX_OT_export_scene_layout)
        bpy.utils.register_class(TKMAX_PT_scene_layout)
        bpy.types.TOPBAR_MT_file_export.append(menu_export)

    def unregister():
        bpy.types.TOPBAR_MT_file_export.remove(menu_export)
        bpy.utils.unregister_class(TKMAX_PT_scene_layout)
        bpy.utils.unregister_class(TKMAX_OT_export_scene_layout)

    if __name__ == "__main__":
        register()
