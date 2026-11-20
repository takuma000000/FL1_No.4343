#include "SceneLayoutData.h"
#include "json.hpp"
#include <array>
#include <algorithm>
#include <cctype>
#include <utility>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

namespace {
	using Json = nlohmann::json;
	void Require(bool value, const std::string& message) { if (!value) { throw std::runtime_error(message); } }
	Vector3 ReadVector(const Json& j) {
		Require(j.is_array() && j.size() == 3, "Expected a three-number vector");
		for (const auto& n : j) { Require(n.is_number(), "Expected a number"); }
		const auto a = j.get<std::array<float, 3>>();
		for (float v : a) { Require(std::isfinite(v) && std::abs(v) <= 1000000.0f, "Invalid transform value"); }
		return { a[0], a[1], a[2] };
	}
	TKM::SceneLayoutTransform ReadTransform(const Json& j) {
		TKM::SceneLayoutTransform t{ ReadVector(j.at("position")), ReadVector(j.at("rotation")), ReadVector(j.at("scale")) };
		Require(std::abs(t.scale.x) >= 0.0001f && std::abs(t.scale.y) >= 0.0001f && std::abs(t.scale.z) >= 0.0001f, "Scale must not be zero");
		return t;
	}
	std::filesystem::path RelativeAsset(const std::filesystem::path& base, const std::string& relative) {
		Require(!relative.empty() && relative.find_first_of("\\ \t\r\n") == std::string::npos, "Asset path must use / and contain no spaces: " + relative);
		const std::filesystem::path path(relative);
		Require(!path.is_absolute() && !path.has_root_name() && relative.find(':') == std::string::npos, "Asset path must be relative");
		const auto full = (base / path).lexically_normal();
		const auto within = full.lexically_relative(std::filesystem::path("resources/obj"));
		Require(!within.empty() && *within.begin() != "..", "Asset must stay under resources/obj");
		Require(std::filesystem::is_regular_file(full), "Missing asset: " + full.generic_string());
		return full;
	}
	void ValidateModel(const std::string& model) {
		const auto path = RelativeAsset("resources/obj", model);
		Require(path.extension() == ".obj", "SceneLayout model must be OBJ: " + model);
		std::ifstream file(path);
		Require(static_cast<bool>(file), "Cannot open model: " + model);
		std::map<std::string, std::string> materials;
		size_t positions = 0, uvs = 0, normals = 0, triangles = 0;
		std::string material, line;
		while (std::getline(file, line)) {
			std::istringstream row(line); std::string id; row >> id;
			if (id == "v" || id == "vn" || id == "vt") {
				const int count = id == "vt" ? 2 : 3;
				for (int i = 0; i < count; ++i) { double v; Require(static_cast<bool>(row >> v) && std::isfinite(v), "Invalid OBJ vector: " + model); }
				if (id == "v") { ++positions; } else if (id == "vt") { ++uvs; } else { ++normals; }
			} else if (id == "mtllib") {
				std::string name; row >> name; const auto mtlPath = RelativeAsset(path.parent_path(), name);
				std::ifstream mtl(mtlPath); Require(static_cast<bool>(mtl), "Cannot open MTL");
				std::string mtlLine, key;
				while (std::getline(mtl, mtlLine)) {
					std::istringstream part(mtlLine); std::string type; part >> type;
					if (type == "newmtl") { part >> key; materials[key] = {}; }
					if (type == "map_Kd") {
						std::string texture; part >> texture;
						const auto texturePath = RelativeAsset(mtlPath.parent_path(), texture);
						auto extension = texturePath.extension().string();
						std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
						Require(extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".dds" || extension == ".bmp", "Unsupported scene layout texture format");
						materials[key] = texture;
					}
				}
			} else if (id == "usemtl") { row >> material; }
			else if (id == "f") {
				Require(materials.contains(material) && !materials.at(material).empty(), "Every face needs a material with map_Kd: " + model);
				const size_t counts[] = { positions, uvs, normals };
				for (int v = 0; v < 3; ++v) {
					std::string vertex; row >> vertex; std::istringstream fields(vertex);
					for (int field = 0; field < 3; ++field) {
						std::string index; std::getline(fields, index, '/'); size_t end = 0;
						const auto value = std::stoll(index, &end);
						Require(end == index.size() && value > 0 && static_cast<size_t>(value) <= counts[field], "Invalid OBJ index: " + model);
					}
					std::string extra; Require(!std::getline(fields, extra, '/'), "Invalid OBJ index fields");
				}
				std::string fourth; row >> fourth; Require(fourth.empty() || fourth[0] == '#', "OBJ faces must be triangles");
				++triangles;
			}
		}
		Require(triangles > 0, "OBJ has no triangles: " + model);
	}
}
namespace TKM {
	bool LoadSceneLayoutData(const std::string& filePath, SceneLayoutData& out, std::string& error) {
		try {
			constexpr uintmax_t kMaxJsonBytes = 8 * 1024 * 1024;
			// char8_tの文字列でUTF-8を明示する（日本語パスにも対応）。
			const std::u8string utf8Path(filePath.begin(), filePath.end());
			const std::filesystem::path path(utf8Path);
			Require(std::filesystem::file_size(path) <= kMaxJsonBytes, "SceneLayout JSON is too large");
			std::ifstream file(path); Require(static_cast<bool>(file), "Cannot open scene layout file");
			Json j; file >> j;
			Require(j.at("format") == "TKMAX_SCENE_LAYOUT" && j.at("version") == 1, "Unsupported scene layout format/version");
			Require(j.at("objects").is_array() && j.at("objects").size() <= 4096, "SceneLayout supports up to 4096 objects");
			SceneLayoutData loaded; std::set<std::string> names, checkedModels;
			for (const auto& item : j.at("objects")) {
				SceneLayoutObjectData object{ item.at("name").get<std::string>(), item.at("model").get<std::string>(), ReadTransform(item), item.value("visible", true) };
				Require(!object.name.empty() && names.insert(object.name).second, "Object names must be nonempty and unique");
				if (checkedModels.insert(object.model).second) { ValidateModel(object.model); }
				loaded.objects.push_back(std::move(object));
			}
			if (j.contains("markers")) {
				Require(j.at("markers").is_array() && j.at("markers").size() <= 4096, "Invalid markers");
				for (const auto& item : j.at("markers")) {
					SceneLayoutMarker marker{ item.at("name").get<std::string>(), item.value("tag", std::string{}), ReadTransform(item) };
					Require(!marker.name.empty() && names.insert(marker.name).second, "Marker names must be unique");
					loaded.markers.push_back(std::move(marker));
				}
			}
			out = std::move(loaded); error.clear(); return true;
		} catch (const std::exception& e) { error = filePath + ": " + e.what(); return false; }
	}
}
