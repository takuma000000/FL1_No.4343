#pragma once
#include "MyMath.h"

#include <functional>
#include <map>
#include <string>

namespace TKM {

	//=============================================================
	// SpriteSaveクラス
	// 登録されたスプライトの設定をJSONへ保存・読込するクラス。
	//=============================================================
	class SpriteSave {
	public:
		struct Values {
			Vector2 position_;
			float rotation_ = 0.0f;
			Vector2 size_;
			Vector4 color_;
			Vector2 anchorPoint_;
			bool isFlipX_ = false;
			bool isFlipY_ = false;
		};

		using Getter = std::function<Values()>;
		using Setter = std::function<void(const Values&)>;

		bool RegisterSprite(
			const std::string& id,
			Getter getter,
			Setter setter
		);

		void UnregisterSprite(const std::string& id);
		void Clear();

		bool SaveSettings();
		bool LoadSettings();

		void SetSettingsPath(const std::string& filePath) {
			settingsPath_ = filePath;
		}

		const std::string& GetSettingsMessage() const {
			return settingsMessage_;
		}

	private:
		struct Entry {
			Getter getter_;
			Setter setter_;
		};

		std::map<std::string, Entry> entries_;
		std::string settingsPath_;
		std::string settingsMessage_;
	};

} // namespace TKM
