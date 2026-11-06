#include "DebugCamera.h"
#include "Input.h"
#include "WindowsAPI.h"
#include <cmath>
#include <algorithm>

#ifdef USE_IMGUI
#include "imgui.h"
#endif

namespace TKM {
	void DebugCamera::Initialize(const Vector3& pos, const Vector3& target) {
		pos_ = pos;

		// pos → target の向きから初期 yaw/pitch を求める
		Vector3 toTarget = target - pos;
		float lenXZ = std::sqrtf(toTarget.x * toTarget.x + toTarget.z * toTarget.z);

		if (lenXZ > 0.0001f) { // XZ平面への射影ベクトルの長さが十分に大きいなら
			yaw_ = std::atan2f(toTarget.x, toTarget.z);
			pitch_ = std::atan2f(toTarget.y, lenXZ);
		}

		SetTranslate(pos_); // カメラ位置セット
		SetRotate({ pitch_, yaw_, 0.0f }); // カメラ回転セット
		Camera::Update();

		// マウス初期位置
		HWND hwnd = GetActiveWindow();
		GetCursorPos(&prevMouse_); // スクリーン座標系
		ScreenToClient(hwnd, &prevMouse_); // クライアント座標系に変換
	}

	void DebugCamera::Update() {
		Input* input = Input::GetInstance();

		// ===================== マウス移動量 =====================
		HWND hwnd = GetActiveWindow();
		POINT cur{};
		GetCursorPos(&cur);          // スクリーン座標系
		ScreenToClient(hwnd, &cur);  // クライアント座標系に変換

		if (firstMouse_) {
			prevMouse_ = cur;
			firstMouse_ = false;
		}

		float dx = float(cur.x - prevMouse_.x);
		float dy = float(cur.y - prevMouse_.y);
		prevMouse_ = cur;

		// ===================== ImGui操作判定 =====================
		bool canControlCamera = true; // ImGui操作中はカメラ操作を無効化する

#ifdef USE_IMGUI
		canControlCamera = !ImGui::GetIO().WantCaptureMouse; // ImGuiがマウス操作を奪っている場合はカメラ操作を無効化
#endif

		// ===================== 操作状態 =====================
		isRotating_ =
			canControlCamera
			&& (GetAsyncKeyState(VK_RBUTTON) & 0x8000);

		isPanning_ =
			canControlCamera
			&& (GetAsyncKeyState(VK_LBUTTON) & 0x8000);

		// ホイール状態は毎フレーム判定し直す
		isZooming_ = false;

		// ===================== 向きベクトル計算 =====================
		Vector3 forward;
		forward.x = std::sinf(yaw_) * std::cosf(pitch_);
		forward.y = std::sinf(pitch_);
		forward.z = std::cosf(yaw_) * std::cosf(pitch_);

		Vector3 right = { forward.z, 0.0f, -forward.x };
		Vector3 up = { 0.0f, 1.0f, 0.0f };

		// ===================== マウスホイールで前後ズーム =====================
		{
			int wheel = input->GetWheel();

			if (wheel != 0) {
				isZooming_ = canControlCamera;

				if (isZooming_) {
					const float wheelZoomSpeed = 1.0f;
					pos_ = pos_
						+ forward * (wheel * wheelZoomSpeed);
				}

				// ImGui操作中でも使用済みの入力はリセットする
				input->SetWheel(0);
			}
		}

		// ===================== 右ドラッグで回転 =====================
		if (isRotating_) {
			const float mouseRotateSpeed = 0.003f;
			yaw_ += dx * mouseRotateSpeed;
			pitch_ += dy * mouseRotateSpeed;

			const float limit = 1.55f;
			pitch_ = std::clamp(pitch_, -limit, limit);
		}

		// ===================== 左ドラッグで座標移動 =====================
		if (isPanning_) {
			const float panSpeed = 0.02f;
			pos_ = pos_
				+ right * (-dx * panSpeed)
				+ up * (dy * panSpeed);
		}

		// ===================== カメラ更新 =====================
		SetTranslate(pos_);
		SetRotate({ pitch_, yaw_, 0.0f });
		Camera::Update();
	}
}