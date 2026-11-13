#pragma once
#include "Sprite.h"
#include "SpriteEditor.h"
#include "SpriteSave.h"

//=============================================================
// SpriteSettingsクラス
// ゲーム内スプライトの保存・編集設定を管理するクラス。
// スプライト本体の所有権は持たない。
//=============================================================
class SpriteSettings {
public:
	/// <summary>
	/// スプライトの保存・編集設定を初期化します。
	/// </summary>
	void Initialize(TKM::Sprite* logoSprite);

	/// <summary>
	/// スプライトの保存・編集設定を終了します。
	/// </summary>
	void Finalize();

	/// <summary>
	/// ImGuiでスプライト編集画面を表示します。
	/// </summary>
	void ImGuiDebug();

private:
	TKM::SpriteSave spriteSave_;
	TKM::SpriteEditor spriteEditor_;
};
