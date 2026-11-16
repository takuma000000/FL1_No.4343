#include "SpriteSave.h"

#include "json.hpp"
#include "WindowsAPI.h"

#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

namespace {

	Vector2 ReadVector2(const nlohmann::json& value) {
		if (!value.is_array() || value.size() != 2) {
			throw std::runtime_error("Expected two numbers");
		}

		const auto numbers = value.get<std::array<float, 2>>();
		if (!std::isfinite(numbers[0]) || !std::isfinite(numbers[1])) {
			throw std::runtime_error("Invalid numeric value");
		}

		return { numbers[0], numbers[1] };
	}

	Vector4 ReadVector4(const nlohmann::json& value) {
		if (!value.is_array() || value.size() != 4) {
			throw std::runtime_error("Expected four numbers");
		}

		const auto numbers = value.get<std::array<float, 4>>();
		for (float number : numbers) {
			if (!std::isfinite(number)) {
				throw std::runtime_error("Invalid numeric value");
			}
		}

		return { numbers[0], numbers[1], numbers[2], numbers[3] };
	}

	void ValidateValues(const TKM::SpriteSave::Values& values) {
		if (!std::isfinite(values.position_.x)
			|| !std::isfinite(values.position_.y)
			|| !std::isfinite(values.rotation_)
			|| !std::isfinite(values.size_.x)
			|| !std::isfinite(values.size_.y)
			|| !std::isfinite(values.anchorPoint_.x)
			|| !std::isfinite(values.anchorPoint_.y)
			|| !std::isfinite(values.color_.x)
			|| !std::isfinite(values.color_.y)
			|| !std::isfinite(values.color_.z)
			|| !std::isfinite(values.color_.w)) {
			throw std::runtime_error("Invalid numeric value");
		}

		if (values.size_.x <= 0.0f || values.size_.y <= 0.0f) {
			throw std::runtime_error("Sprite size must be greater than zero");
		}
	}

}

namespace TKM {

	bool SpriteSave::RegisterSprite(
		const std::string& id,
		Getter getter,
		Setter setter
	) {
		if (id.empty() || !getter || !setter) {
			return false;
		}

		return entries_.emplace(
			id,
			Entry{ std::move(getter), std::move(setter) }
		).second;
	}

	void SpriteSave::UnregisterSprite(const std::string& id) {
		entries_.erase(id);
	}

	void SpriteSave::Clear() {
		entries_.clear();
	}

	bool SpriteSave::SaveSettings() {
		try {
			if (settingsPath_.empty()) {
				throw std::runtime_error("Settings path is not set");
			}

			nlohmann::json sprites = nlohmann::json::object();

			for (const auto& [id, entry] : entries_) {
				const Values values = entry.getter_();
				ValidateValues(values);

				sprites[id] = {
					{ "position", { values.position_.x, values.position_.y } },
					{ "rotation", values.rotation_ },
					{ "size", { values.size_.x, values.size_.y } },
					{ "color", {
						values.color_.x,
						values.color_.y,
						values.color_.z,
						values.color_.w
					} },
					{ "anchor", {
						values.anchorPoint_.x,
						values.anchorPoint_.y
					} },
					{ "flipX", values.isFlipX_ },
					{ "flipY", values.isFlipY_ }
				};
			}

			const nlohmann::json data = {
				{ "version", 1 },
				{ "sprites", std::move(sprites) }
			};

			const std::filesystem::path path(settingsPath_);
			if (!path.parent_path().empty()) {
				std::filesystem::create_directories(path.parent_path());
			}

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

			settingsMessage_ = "スプライトを保存しました: " + settingsPath_;
			return true;
		}
		catch (const std::exception& error) {
			settingsMessage_ =
				"保存失敗: " + settingsPath_ + "\n" + error.what();
			return false;
		}
	}

	bool SpriteSave::SaveSettings(const std::string& id) {
		try {
			if (settingsPath_.empty()) {
				throw std::runtime_error("Settings path is not set");
			}

			const auto entryIterator = entries_.find(id);
			if (entryIterator == entries_.end()) {
				throw std::runtime_error("Sprite is not registered: " + id);
			}

			nlohmann::json data = {
				{ "version", 1 },
				{ "sprites", nlohmann::json::object() }
			};

			// 既存データを読み込み、選択対象以外の保存値を維持する
			if (std::filesystem::exists(settingsPath_)) {
				std::ifstream inputFile(settingsPath_);
				if (!inputFile) {
					throw std::runtime_error("Cannot open settings file");
				}

				inputFile >> data;

				if (data.at("version").get<int>() != 1) {
					throw std::runtime_error("Unsupported settings version");
				}

				if (!data.at("sprites").is_object()) {
					throw std::runtime_error("Sprites must be a JSON object");
				}
			}

			const Values values = entryIterator->second.getter_();
			ValidateValues(values);

			data["sprites"][id] = {
				{ "position", { values.position_.x, values.position_.y } },
				{ "rotation", values.rotation_ },
				{ "size", { values.size_.x, values.size_.y } },
				{ "color", {
					values.color_.x,
					values.color_.y,
					values.color_.z,
					values.color_.w
				} },
				{ "anchor", {
					values.anchorPoint_.x,
					values.anchorPoint_.y
				} },
				{ "flipX", values.isFlipX_ },
				{ "flipY", values.isFlipY_ }
			};

			const std::filesystem::path path(settingsPath_);
			if (!path.parent_path().empty()) {
				std::filesystem::create_directories(path.parent_path());
			}

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

			settingsMessage_ =
				"選択中のスプライトを保存しました: " + id;
			return true;
		}
		catch (const std::exception& error) {
			settingsMessage_ =
				"保存失敗: " + id + "\n" + error.what();
			return false;
		}
	}

	bool SpriteSave::LoadSettings() {
		try {
			if (settingsPath_.empty()) {
				throw std::runtime_error("Settings path is not set");
			}

			if (!std::filesystem::exists(settingsPath_)) {
				settingsMessage_ = "保存ファイルなし: 現在の値を使用します";
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

			const nlohmann::json& sprites = data.at("sprites");
			if (!sprites.is_object()) {
				throw std::runtime_error("Sprites must be a JSON object");
			}

			std::map<std::string, Values> loadedValues;

			for (const auto& [id, entry] : entries_) {
				if (!sprites.contains(id)) {
					continue;
				}

				const nlohmann::json& spriteData = sprites.at(id);
				Values values{};
				values.position_ = ReadVector2(spriteData.at("position"));
				values.rotation_ = spriteData.at("rotation").get<float>();
				values.size_ = ReadVector2(spriteData.at("size"));
				values.color_ = ReadVector4(spriteData.at("color"));
				values.anchorPoint_ = ReadVector2(spriteData.at("anchor"));
				values.isFlipX_ = spriteData.at("flipX").get<bool>();
				values.isFlipY_ = spriteData.at("flipY").get<bool>();

				ValidateValues(values);
				loadedValues.emplace(id, values);
			}

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

	bool SpriteSave::LoadSettings(const std::string& id) {
		try {
			if (settingsPath_.empty()) {
				throw std::runtime_error("Settings path is not set");
			}

			const auto entryIterator = entries_.find(id);
			if (entryIterator == entries_.end()) {
				throw std::runtime_error("Sprite is not registered: " + id);
			}

			if (!std::filesystem::exists(settingsPath_)) {
				throw std::runtime_error("Settings file does not exist");
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

			const nlohmann::json& sprites = data.at("sprites");
			if (!sprites.is_object()) {
				throw std::runtime_error("Sprites must be a JSON object");
			}

			if (!sprites.contains(id)) {
				throw std::runtime_error("Saved sprite was not found: " + id);
			}

			const nlohmann::json& spriteData = sprites.at(id);
			Values values{};
			values.position_ = ReadVector2(spriteData.at("position"));
			values.rotation_ = spriteData.at("rotation").get<float>();
			values.size_ = ReadVector2(spriteData.at("size"));
			values.color_ = ReadVector4(spriteData.at("color"));
			values.anchorPoint_ = ReadVector2(spriteData.at("anchor"));
			values.isFlipX_ = spriteData.at("flipX").get<bool>();
			values.isFlipY_ = spriteData.at("flipY").get<bool>();

			ValidateValues(values);
			entryIterator->second.setter_(values);

			settingsMessage_ =
				"選択中のスプライトを保存状態に戻しました: " + id;
			return true;
		}
		catch (const std::exception& error) {
			settingsMessage_ =
				"読込失敗: " + id + "\n" + error.what();
			return false;
		}
	}

} // namespace TKM
