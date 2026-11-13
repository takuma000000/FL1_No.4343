#include "SpriteSettings.h"

void SpriteSettings::Initialize(
	TKM::Sprite* logoSprite,
	TKM::Sprite* uvCheckerSprite
) {
	// GAMEシーンのスプライト保存先を設定する
	spriteSave_.SetSettingsPath(
		"resources/data/game_sprite.json"
	);

	//======================================================================
	// TKMAX Logo
	//======================================================================
	if (logoSprite) {
		// 保存対象へ登録する
		spriteSave_.RegisterSprite(
			"TKMAX Logo",
			[logoSprite]() {
				TKM::SpriteSave::Values values{};
				values.position_ = logoSprite->GetPosition();
				values.rotation_ = logoSprite->GetRotation();
				values.size_ = logoSprite->GetSize();
				values.color_ = logoSprite->GetColor();
				values.anchorPoint_ = logoSprite->GetAnchorPoint();
				values.isFlipX_ = logoSprite->GetIsFlipX();
				values.isFlipY_ = logoSprite->GetIsFlipY();
				return values;
			},
			[logoSprite](const TKM::SpriteSave::Values& values) {
				logoSprite->SetPosition(values.position_);
				logoSprite->SetRotation(values.rotation_);
				logoSprite->SetSize(values.size_);
				logoSprite->SetColor(values.color_);
				logoSprite->SetAnchorPoint(values.anchorPoint_);
				logoSprite->SetIsFlipX(values.isFlipX_);
				logoSprite->SetIsFlipY(values.isFlipY_);
			}
		);

#ifdef USE_IMGUI
		// ImGuiの編集対象へ登録する
		spriteEditor_.RegisterSprite(
			"TKMAX Logo",
			logoSprite,
			[logoSprite]() {
				logoSprite->ImGuiDebug();
			}
		);
#endif
	}

	//======================================================================
	// UV Checker
	//======================================================================
	if (uvCheckerSprite) {
		// 保存対象へ登録する
		spriteSave_.RegisterSprite(
			"UV Checker",
			[uvCheckerSprite]() {
				TKM::SpriteSave::Values values{};
				values.position_ = uvCheckerSprite->GetPosition();
				values.rotation_ = uvCheckerSprite->GetRotation();
				values.size_ = uvCheckerSprite->GetSize();
				values.color_ = uvCheckerSprite->GetColor();
				values.anchorPoint_ = uvCheckerSprite->GetAnchorPoint();
				values.isFlipX_ = uvCheckerSprite->GetIsFlipX();
				values.isFlipY_ = uvCheckerSprite->GetIsFlipY();
				return values;
			},
			[uvCheckerSprite](const TKM::SpriteSave::Values& values) {
				uvCheckerSprite->SetPosition(values.position_);
				uvCheckerSprite->SetRotation(values.rotation_);
				uvCheckerSprite->SetSize(values.size_);
				uvCheckerSprite->SetColor(values.color_);
				uvCheckerSprite->SetAnchorPoint(values.anchorPoint_);
				uvCheckerSprite->SetIsFlipX(values.isFlipX_);
				uvCheckerSprite->SetIsFlipY(values.isFlipY_);
			}
		);

#ifdef USE_IMGUI
		// ImGuiの編集対象へ登録する
		spriteEditor_.RegisterSprite(
			"UV Checker",
			uvCheckerSprite,
			[uvCheckerSprite]() {
				uvCheckerSprite->ImGuiDebug();
			}
		);
#endif
	}

#ifdef USE_IMGUI
	spriteEditor_.SetSpriteSave(&spriteSave_);
#endif

	// 全スプライトの登録後に保存値を読み込む
	spriteSave_.LoadSettings();
}

void SpriteSettings::Finalize() {
	spriteEditor_.Clear();
	spriteSave_.Clear();
}

void SpriteSettings::ImGuiDebug() {
#ifdef USE_IMGUI
	spriteEditor_.ImGuiDebug();
#endif
}
