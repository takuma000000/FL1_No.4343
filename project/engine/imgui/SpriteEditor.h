#pragma once
#include "SpriteGizmo.h"
#include "SpriteSave.h"

#include <functional>
#include <string>
#include <vector>

namespace TKM {

	//=============================================================
	// SpriteEditorクラス
	// 登録されたスプライトの選択とImGui編集を管理するクラス。
	//=============================================================
	class SpriteEditor {
	public:
		using ImGuiCallback = std::function<void()>;

		void SetSpriteSave(SpriteSave* spriteSave);

		bool RegisterSprite(
			const std::string& name,
			Sprite* sprite,
			ImGuiCallback callback
		);

		void UnregisterSprite(const std::string& name);
		void Clear();
		void ImGuiDebug();

	private:
		struct Entry {
			std::string name_;
			Sprite* sprite_ = nullptr;
			ImGuiCallback callback_;
		};

		std::vector<Entry> entries_;
		int selectedIndex_ = 0;
		SpriteSave* spriteSave_ = nullptr;
		SpriteGizmo spriteGizmo_;
	};

} // namespace TKM
