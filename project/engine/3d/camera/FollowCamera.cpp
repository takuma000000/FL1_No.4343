#include "FollowCamera.h"
#include "json.hpp"
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace {
	Vector3 ReadFollowVector(const nlohmann::json& value) {
		if (!value.is_array() || value.size() != 3) {
			throw std::runtime_error("Expected three numbers");
		}
		const auto values = value.get<std::array<float, 3>>();
		for (float number : values) {
			if (!std::isfinite(number)) {
				throw std::runtime_error("Invalid numeric value");
			}
		}
		return { values[0], values[1], values[2] };
	}
}

namespace TKM {
	void FollowCamera::Initialize(Camera* camera, const Vector3& offset) {
		camera_ = camera;
		offset_ = offset;
		rotation_ = camera ? camera->GetRotate() : Vector3{ 0.0f, 0.0f, 0.0f };
		LoadSettings(); // 保存ファイルがなければ呼び出し側の初期値を使う
	}

	void FollowCamera::Update(const Vector3& target) {
		if (!camera_) { return; }
		camera_->SetTranslate({ target.x + offset_.x, target.y + offset_.y, target.z + offset_.z });
		camera_->SetRotate(rotation_);
		camera_->Update();
	}

	void FollowCamera::ImGuiDebug(const Vector3& target) {
#ifdef USE_IMGUI
		ImGui::Begin("追従カメラ");
		ImGui::TextWrapped("Playerの位置にオフセットを加えて追従します。回転の単位はラジアンです。");
		bool changed = ImGui::DragFloat3("位置オフセット", &offset_.x, 0.1f);
		changed |= ImGui::DragFloat3("回転", &rotation_.x, 0.01f);
		// 距離はオフセットの長さ。同じ値を二重に保存せず、方向を保って伸縮する。
		const float length = std::hypot(offset_.x, offset_.y, offset_.z);
		float distance = length;
		if (ImGui::DragFloat("Playerとの距離", &distance, 0.1f, 0.0f, 0.0f)) {
			if (!std::isfinite(distance) || distance < 0.0f) { distance = 0.0f; }
			if (length > 0.0f) {
				const float ratio = distance / length;
				offset_ = { offset_.x * ratio, offset_.y * ratio, offset_.z * ratio };
			} else {
				offset_ = { 0.0f, 0.0f, -distance };
			}
			changed = true;
		}
		ImGui::Separator();
		ImGui::TextUnformatted(kSettingsPath);
		if (ImGui::Button("追従設定を保存")) { SaveSettings(); }
		ImGui::SameLine();
		if (ImGui::Button("最後の保存状態に戻す")) { changed |= LoadSettings(); }
		if (!settingsMessage_.empty()) { ImGui::TextWrapped("%s", settingsMessage_.c_str()); }
		if (changed) { Update(target); } // 同じフレームの描画へ反映
		ImGui::End();
#else
		(void)target;
#endif
	}

	bool FollowCamera::SaveSettings() {
		try {
			// 読込時と同じ検証を行い、不正値で既存ファイルを壊さない。
			nlohmann::json data = {
				{ "version", 1 },
				{ "offset", { offset_.x, offset_.y, offset_.z } },
				{ "rotation", { rotation_.x, rotation_.y, rotation_.z } }
			};
			ReadFollowVector(data.at("offset"));
			ReadFollowVector(data.at("rotation"));
			const std::filesystem::path path(kSettingsPath);
			std::filesystem::create_directories(path.parent_path());
			auto temporaryPath = path;
			temporaryPath += ".tmp";
			std::ofstream file(temporaryPath, std::ios::trunc);
			if (!file) { throw std::runtime_error("Cannot open temporary file"); }
			file << data.dump(4);
			file.close();
			if (!file) { throw std::runtime_error("Failed to write settings"); }
			if (!MoveFileExW(temporaryPath.c_str(), path.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
				throw std::runtime_error("Cannot replace settings file: " + std::to_string(GetLastError()));
			}
			settingsMessage_ = "追従設定を保存しました";
			return true;
		} catch (const std::exception& error) {
			settingsMessage_ = std::string("保存失敗: ") + error.what();
			return false;
		}
	}

	bool FollowCamera::LoadSettings() {
		try {
			if (!std::filesystem::exists(kSettingsPath)) {
				settingsMessage_ = "保存ファイルなし: 初期値を使用します";
				return false;
			}
			std::ifstream file(kSettingsPath);
			if (!file) { throw std::runtime_error("Cannot open settings file"); }
			nlohmann::json data;
			file >> data;
			if (data.at("version").get<int>() != 1) {
				throw std::runtime_error("Unsupported settings version");
			}
			// 両方の検証に成功するまで現在の設定を変更しない。
			const Vector3 offset = ReadFollowVector(data.at("offset"));
			const Vector3 rotation = ReadFollowVector(data.at("rotation"));
			offset_ = offset;
			rotation_ = rotation;
			settingsMessage_ = "追従設定を読み込みました";
			return true;
		} catch (const std::exception& error) {
			settingsMessage_ = std::string("読込失敗: ") + error.what();
			return false;
		}
	}
}
