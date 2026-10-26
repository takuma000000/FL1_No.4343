#include "Camera.h"

#include "json.hpp"

#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <stdexcept>

/// カメラの設定を保存・読み込みする
namespace {
	// JSONの配列をVector3に変換する
	Vector3 ReadCameraVector3(const nlohmann::json& value) {
		if (!value.is_array() || value.size() != 3) {
			throw std::runtime_error("Expected three numbers");
		}

		const auto numbers = value.get<std::array<float, 3>>();

		for (float number : numbers) {
			if (!std::isfinite(number)) {
				throw std::runtime_error("Invalid numeric value");
			}
		}

		return { numbers[0], numbers[1], numbers[2] };
	}

	// カメラ設定として使用できる数値か確認する
	void ValidateCameraSettings(
		const Vector3& position,
		const Vector3& rotation,
		const Vector3& scale
	) {
		auto isFinite = [](const Vector3& value) {
			return std::isfinite(value.x)
				&& std::isfinite(value.y)
				&& std::isfinite(value.z);
			};

		if (!isFinite(position)
			|| !isFinite(rotation)
			|| !isFinite(scale)) {
			throw std::runtime_error("Invalid numeric value");
		}

		// スケール0では逆行列を計算できない
		if (scale.x == 0.0f || scale.y == 0.0f || scale.z == 0.0f) {
			throw std::runtime_error("Camera scale must not be zero");
		}
	}
}

namespace TKM {
	Camera::Camera()
		//初期化
		:transform_({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} })
		, fovY_(0.45f)
		, aspectRatio_(float(WindowsAPI::GetClientWidth()) / float(WindowsAPI::GetClientHeight()))
		, nearClip_(0.1f)
		, farClip_(200.0f) // 描画距離の上限
		, worldMatrix_(MyMath::MakeAffineMatrix(transform_.scale_, transform_.rotate_, transform_.translate_))
		, viewMatrix_(MyMath::Inverse4x4(worldMatrix_))
		, projectionMatrix_(MyMath::MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_))
		, viewProjectionMatrix_(MyMath::Multiply(viewMatrix_, projectionMatrix_)) {}

	void Camera::Update() {
		//cameraTransformからcameraMatrixを作る
		worldMatrix_ = MyMath::MakeAffineMatrix(transform_.scale_, transform_.rotate_, transform_.translate_);
		//cameraMatrixからviewMatrixを作る
		viewMatrix_ = MyMath::Inverse4x4(worldMatrix_);
		//projectionMatrixを作って投資投影行列を書き込む
		projectionMatrix_ = MyMath::MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
		//合成行列
		viewProjectionMatrix_ = MyMath::Multiply(viewMatrix_, projectionMatrix_);
	}

	void Camera::ImGuiDebug() {
#ifdef USE_IMGUI
		ImGui::Begin("カメラ");
		ImGui::DragFloat3("位置", &transform_.translate_.x, 0.01f);
		ImGui::DragFloat3("回転", &transform_.rotate_.x, 0.01f);
		ImGui::DragFloat3("拡縮", &transform_.scale_.x, 0.01f);

		// 保存・読込
		ImGui::Separator();

		if (ImGui::Button("保存")) {
			SaveSettings();
		}

		ImGui::SameLine();

		if (ImGui::Button("最後の保存状態に戻す")) {
			LoadSettings();
		}

		if (!settingsMessage_.empty()) {
			ImGui::TextWrapped("%s", settingsMessage_.c_str());
		}

		ImGui::End();
#endif
	}

	bool Camera::SaveSettings() {
		try {
			if (settingsPath_.empty()) {
				throw std::runtime_error("Settings path is not set");
			}

			ValidateCameraSettings(
				transform_.translate_,
				transform_.rotate_,
				transform_.scale_
			);

			nlohmann::json data = {
				{ "version", 1 },
				{ "position", {
					transform_.translate_.x,
					transform_.translate_.y,
					transform_.translate_.z
				}},
				{ "rotation", {
					transform_.rotate_.x,
					transform_.rotate_.y,
					transform_.rotate_.z
				}},
				{ "scale", {
					transform_.scale_.x,
					transform_.scale_.y,
					transform_.scale_.z
				}}
			};

			const std::filesystem::path path(settingsPath_);

			// 保存先フォルダがなければ作成する
			if (!path.parent_path().empty()) {
				std::filesystem::create_directories(path.parent_path());
			}

			// 一時ファイルに書き込んでから保存先を置き換える
			auto temporaryPath = path;
			temporaryPath += ".tmp";

			std::ofstream file(temporaryPath, std::ios::trunc);
			if (!file) {
				throw std::runtime_error("Cannot open temporary file");
			}

			file << data.dump(4);
			file.close();

			if (!file) {
				throw std::runtime_error("Failed to write settings");
			}

			if (!MoveFileExW(
				temporaryPath.c_str(),
				path.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
			)) {
				throw std::runtime_error(
					"Cannot replace settings file. Windows error: "
					+ std::to_string(GetLastError())
				);
			}

			settingsMessage_ = "保存したよ！！！！！: " + settingsPath_;
			return true;
		}
		catch (const std::exception& error) {
			settingsMessage_ =
				"保存失敗: " + settingsPath_ + "\n" + error.what();
			return false;
		}
	}

	bool Camera::LoadSettings() {
		try {
			if (settingsPath_.empty()) {
				throw std::runtime_error("Settings path is not set");
			}

			// ファイルがなければ現在の初期値を使用する
			if (!std::filesystem::exists(settingsPath_)) {
				settingsMessage_ = "保存ファイルなし: 初期値を使用します";
				return false;
			}

			std::ifstream file(settingsPath_);
			if (!file) {
				throw std::runtime_error("Cannot open settings file");
			}

			nlohmann::json data;
			file >> data;

			if (data.at("version").get<int>() != 1) {
				throw std::runtime_error("Unsupported settings version");
			}

			// 全項目を確認してからカメラに反映する
			const Vector3 position = ReadCameraVector3(data.at("position"));
			const Vector3 rotation = ReadCameraVector3(data.at("rotation"));
			const Vector3 scale = ReadCameraVector3(data.at("scale"));

			ValidateCameraSettings(position, rotation, scale);

			transform_.translate_ = position;
			transform_.rotate_ = rotation;
			transform_.scale_ = scale;

			Update();

			settingsMessage_ = "最後の保存状態に戻したよ！！！: " + settingsPath_;
			return true;
		}
		catch (const std::exception& error) {
			settingsMessage_ =
				"読込失敗: " + settingsPath_ + "\n" + error.what();
			return false;
		}
	}
}