#define NOMINMAX
#include "ObjectGizmo.h"
#include "ObjectGizmoMath.h"
#include <algorithm>
#include <cmath>
#ifdef USE_IMGUI
#include "imgui.h"
#include "../../externals/ImGuizmo/ImGuizmo.h"
#endif

namespace TKM {
	void ObjectGizmo::SetEnabled(bool enabled) {
		enabled_ = enabled;
#ifdef USE_IMGUI
		if (!enabled) {
			ImGuizmo::PushID(currentId_);
			ImGuizmo::Enable(false);
			ImGuizmo::PopID();
		}
#endif
	}
	void ObjectGizmo::Clear() { SetEnabled(false); }
	bool ObjectGizmo::IsBusy() const {
#ifdef USE_IMGUI
		return enabled_ && (ImGuizmo::IsOver() || ImGuizmo::IsUsingAny());
#else
		return false;
#endif
	}
	bool ObjectGizmo::CanPick() const {
#ifdef USE_IMGUI
		return enabled_ && !ImGuizmo::IsUsingAny() && !ImGuizmo::IsOver()
			&& !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)
			&& !ImGui::IsAnyItemActive() && !ImGui::GetIO().WantCaptureMouse
			&& ImGui::IsMouseClicked(ImGuiMouseButton_Left);
#else
		return false;
#endif
	}
	void ObjectGizmo::ImGuiControls() {
#ifdef USE_IMGUI
		ImGui::SeparatorText("3Dギズモ");
		if (ImGui::Checkbox("画面で3Dオブジェクトを編集", &enabled_)) { SetEnabled(enabled_); }
		if (!enabled_) { return; }
		const char* modes[] = { "移動", "回転", "拡縮" };
		ImGui::Combo("操作", &operation_, modes, IM_ARRAYSIZE(modes));
		ImGui::Checkbox("ローカル軸（OFF: ワールド軸）", &local_);
		ImGui::Checkbox("スナップ", &snap_);
		if (snap_) {
			if (operation_ == 0) { ImGui::DragFloat("移動の刻み", &moveSnap_, 0.1f, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp); }
			if (operation_ == 1) { ImGui::DragFloat("回転の刻み（度）", &rotateSnap_, 1.0f, 1.0f, 90.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp); }
			if (operation_ == 2) { ImGui::DragFloat("拡縮の刻み", &scaleSnap_, 0.01f, 0.01f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp); }
		}
		ImGui::TextWrapped("モデルを左クリックして選択。ハンドルをドラッグして編集できます。拡縮はローカル軸です。");
		ImGui::TextWrapped("位置編集はデバッグカメラ推奨。追従カメラはPlayerと一緒に動きます。");
		ImGui::TextWrapped("保存は保存ボタンから。ゲーム操作キーを押すとPlayerも動くので、マウスで編集してください。");
#endif
	}
	bool ObjectGizmo::Manipulate(ObjectSave::Values& values, const Camera& camera, int id) {
#ifdef USE_IMGUI
		if (!enabled_ || (values.visible_.has_value() && !*values.visible_)) { return false; }
		currentId_ = id;
		ImGuizmo::PushID(id);
		struct IdScope { ~IdScope() { ImGuizmo::PopID(); } } idScope;
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList(ImGui::GetMainViewport()));
		const auto* viewport = ImGui::GetMainViewport();
		ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);
		const bool overWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemActive();
		if (overWindow && !ImGuizmo::IsUsing()) { ImGuizmo::Enable(false); } else { ImGuizmo::Enable(true); }
		auto world = MyMath::MakeAffineMatrix(values.scale_, values.rotation_, values.position_);
		const ImGuizmo::OPERATION operations[] = { ImGuizmo::TRANSLATE, ImGuizmo::ROTATE, ImGuizmo::SCALE };
		const float step = operation_ == 0 ? moveSnap_ : operation_ == 1 ? rotateSnap_ : scaleSnap_;
		const float snapValues[] = { step, step, step };
		const bool changed = ImGuizmo::Manipulate(&camera.GetViewMatrix().m[0][0], &camera.GetProjectionMatrix().m[0][0],
			operations[operation_], local_ || operation_ == 2 ? ImGuizmo::LOCAL : ImGuizmo::WORLD,
			&world.m[0][0], nullptr, snap_ ? snapValues : nullptr);
		if (IsBusy()) {
			ImGui::GetIO().WantCaptureMouse = true; // 今フレームのデバッグカメラ・スプライト操作も止める
			ImGui::SetNextFrameWantCaptureMouse(true);
		}
		if (!changed) { return false; }
		if (operation_ == 0) { values.position_ = { world.m[3][0], world.m[3][1], world.m[3][2] }; }
		if (operation_ == 1 && !ObjectGizmoMath::ReadRotation(world, values.scale_, values.rotation_)) { return false; }
		if (operation_ == 2) {
			const auto axisSize = [&](int i, float oldValue) {
				return std::copysign(std::max(0.001f, std::sqrt(world.m[i][0]*world.m[i][0]+world.m[i][1]*world.m[i][1]+world.m[i][2]*world.m[i][2])), oldValue);
			};
			values.scale_ = { axisSize(0, values.scale_.x), axisSize(1, values.scale_.y), axisSize(2, values.scale_.z) };
		}
		return true;
#else
		(void)values; (void)camera; (void)id; return false;
#endif
	}
}
