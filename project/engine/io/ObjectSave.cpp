#include "ObjectSave.h"

#include "json.hpp"
#include "WindowsAPI.h"

#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

namespace {

	// JSONの配列をVector3に変換する
	Vector3 ReadObjectVector3(const nlohmann::json& value) {
		if (!value.is_array() || value.size() != 3) {
			throw std::runtime_error("Expected three numbers");
		}

		const auto numbers = value.get<std::array<float, 3>>();

		for (float number : numbers) {
			if (!std::isfinite(number)) {
				throw std::runtime_error("Invalid numeric value");
			}
		}

		return {
			numbers[0],
			numbers[1],
			numbers[2]
		};
	}

	// Vector3のすべての数値が正常か確認する
	bool IsFiniteVector3(const Vector3& value) {
		return std::isfinite(value.x)
			&& std::isfinite(value.y)
			&& std::isfinite(value.z);
	}

	// オブジェクト設定として使用できる値か確認する
	void ValidateObjectValues(const TKM::ObjectSave::Values& values) {
		if (!IsFiniteVector3(values.position_)
			|| !IsFiniteVector3(values.rotation_)
			|| !IsFiniteVector3(values.scale_)) {
			throw std::runtime_error("Invalid numeric value");
		}

		// スケール0では正常な行列を作れない
		if (values.scale_.x == 0.0f
			|| values.scale_.y == 0.0f
			|| values.scale_.z == 0.0f) {
			throw std::runtime_error("Object scale must not be zero");
		}
	}

}

namespace TKM {

	bool ObjectSave::RegisterObject(
		const std::string& id,
		Getter getter,
		Setter setter
	) {
		// 名前や処理が設定されていなければ登録しない
		if (id.empty() || !getter || !setter) {
			return false;
		}

		// 同じ名前がすでに登録されている場合も登録しない
		return entries_.emplace(
			id,
			Entry{
				std::move(getter),
				std::move(setter)
			}
		).second;
	}

	void ObjectSave::UnregisterObject(const std::string& id) {
		entries_.erase(id);
	}

	void ObjectSave::Clear() {
		entries_.clear();
	}

	bool ObjectSave::SaveSettings() {
		try {
			if (settingsPath_.empty()) {
				throw std::runtime_error("Settings path is not set");
			}

			nlohmann::json objects = nlohmann::json::object();

			// 登録されたすべてのオブジェクトから現在値を取得する
			for (const auto& [id, entry] : entries_) {
				const Values values = entry.getter_();

				ValidateObjectValues(values);

				nlohmann::json objectData = {
					{ "position", {
						values.position_.x,
						values.position_.y,
						values.position_.z
					}},
					{ "rotation", {
						values.rotation_.x,
						values.rotation_.y,
						values.rotation_.z
					}},
					{ "scale", {
						values.scale_.x,
						values.scale_.y,
						values.scale_.z
					}}
				};

				// 表示状態を持っている場合だけ保存する
				if (values.visible_.has_value()) {
					objectData["visible"] = values.visible_.value();
				}

				objects[id] = std::move(objectData);
			}

			const nlohmann::json data = {
				{ "version", 1 },
				{ "objects", std::move(objects) }
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

			settingsMessage_ = "オブジェクトを保存しました: " + settingsPath_;
			return true;
		}
		catch (const std::exception& error) {
			settingsMessage_ =
				"保存失敗: " + settingsPath_ + "\n" + error.what();
			return false;
		}
	}

	bool ObjectSave::LoadSettings() {
		try {
			if (settingsPath_.empty()) {
				throw std::runtime_error("Settings path is not set");
			}

			// ファイルがなければ現在の値を使用する
			if (!std::filesystem::exists(settingsPath_)) {
				settingsMessage_ =
					"保存ファイルなし: 現在の値を使用します";
				return false;
			}

			std::ifstream file(settingsPath_);
			if (!file) {
				throw std::runtime_error("Cannot open settings file");
			}

			nlohmann::json data;
			file >> data;

			if (data.at("version").get<int>() != 1) {
				throw std::runtime_error(
					"Unsupported settings version"
				);
			}

			const nlohmann::json& objects = data.at("objects");
			if (!objects.is_object()) {
				throw std::runtime_error(
					"Objects must be a JSON object"
				);
			}

			// 全項目を確認してからオブジェクトへ反映する
			std::map<std::string, Values> loadedValues;

			for (const auto& [id, entry] : entries_) {
				// 登録されているがJSONに存在しないオブジェクトは変更しない
				if (!objects.contains(id)) {
					continue;
				}

				const nlohmann::json& objectData = objects.at(id);
				if (!objectData.is_object()) {
					throw std::runtime_error(
						"Invalid object data: " + id
					);
				}

				Values values{};

				values.position_ =
					ReadObjectVector3(objectData.at("position"));

				values.rotation_ =
					ReadObjectVector3(objectData.at("rotation"));

				values.scale_ =
					ReadObjectVector3(objectData.at("scale"));

				// visibleが保存されている場合だけ読み込む
				if (objectData.contains("visible")) {
					if (!objectData.at("visible").is_boolean()) {
						throw std::runtime_error(
							"Invalid visible value: " + id
						);
					}

					values.visible_ =
						objectData.at("visible").get<bool>();
				}

				ValidateObjectValues(values);
				loadedValues.emplace(id, values);
			}

			// JSON内の値がすべて正常だった場合だけ反映する
			for (const auto& [id, values] : loadedValues) {
				entries_.at(id).setter_(values);
			}

			settingsMessage_ =
				"最後の保存状態に戻しました: " + settingsPath_;
			return true;
		}
		catch (const std::exception& error) {
			settingsMessage_ =
				"読込失敗: " + settingsPath_ + "\n" + error.what();
			return false;
		}
	}

} // namespace TKM