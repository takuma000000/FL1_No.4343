#pragma once
#include "Camera.h"

namespace TKM {
	// 最終座標ではなく、追従に使う相対位置と回転を編集・保存する。
	// Cameraの寿命はこのクラスより長く保つこと。
	class FollowCamera {
	public:
		void Initialize(Camera* camera, const Vector3& offset);
		void Update(const Vector3& target);
		void ImGuiDebug(const Vector3& target);
		bool SaveSettings();
		bool LoadSettings();

	private:
		Camera* camera_ = nullptr;
		Vector3 offset_ = { 0.0f, 0.0f, -60.0f };
		Vector3 rotation_ = { 0.0f, 0.0f, 0.0f };
		std::string settingsMessage_;
		static constexpr const char* kSettingsPath = "resources/data/followCamera.json";
	};
}
