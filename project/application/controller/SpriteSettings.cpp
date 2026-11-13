#include "SpriteSettings.h"

#include <algorithm>

void SpriteSettings::Initialize() {
	// GAMEシーンのスプライト保存先を設定する
	spriteSave_.SetSettingsPath(
		"resources/data/game_sprite.json"
	);

#ifdef USE_IMGUI
	spriteEditor_.SetSpriteSave(&spriteSave_);
#endif

	// 現在存在する全スプライトを自動登録する
	SynchronizeSprites();

	// 全スプライトの登録後に保存値を読み込む
	spriteSave_.LoadSettings();
}

void SpriteSettings::Finalize() {
	registeredSprites_.clear();
	spriteEditor_.Clear();
	spriteSave_.Clear();
}

void SpriteSettings::ImGuiDebug() {
#ifdef USE_IMGUI
	// 実行中に追加・削除されたスプライトも自動で反映する
	SynchronizeSprites();
	spriteEditor_.ImGuiDebug();
#endif
}

void SpriteSettings::SynchronizeSprites() {
	const auto& activeSprites = TKM::Sprite::GetActiveSprites();

	// 破棄済みのスプライトを編集・保存対象から解除する
	for (auto iterator = registeredSprites_.begin();
		iterator != registeredSprites_.end();) {
		TKM::Sprite* sprite = iterator->first;
		const std::string id = iterator->second;

		if (std::find(activeSprites.begin(), activeSprites.end(), sprite)
			== activeSprites.end()
			|| sprite->GetEditorName() != id
			|| !sprite->IsEditorEnabled()) {
			spriteSave_.UnregisterSprite(id);
#ifdef USE_IMGUI
			spriteEditor_.UnregisterSprite(id);
#endif
			iterator = registeredSprites_.erase(iterator);
		} else {
			++iterator;
		}
	}

	// 新しく生成されたスプライトを編集・保存対象へ登録する
	for (TKM::Sprite* sprite : activeSprites) {
		if (!sprite
			|| sprite->GetEditorName().empty()
			|| !sprite->IsEditorEnabled()
			|| registeredSprites_.contains(sprite)) {
			continue;
		}

		const std::string id = sprite->GetEditorName();

		const bool saveRegistered = spriteSave_.RegisterSprite(
			id,
			[sprite]() {
				TKM::SpriteSave::Values values{};
				values.position_ = sprite->GetPosition();
				values.rotation_ = sprite->GetRotation();
				values.size_ = sprite->GetSize();
				values.color_ = sprite->GetColor();
				values.anchorPoint_ = sprite->GetAnchorPoint();
				values.isFlipX_ = sprite->GetIsFlipX();
				values.isFlipY_ = sprite->GetIsFlipY();
				return values;
			},
			[sprite](const TKM::SpriteSave::Values& values) {
				sprite->SetPosition(values.position_);
				sprite->SetRotation(values.rotation_);
				sprite->SetSize(values.size_);
				sprite->SetColor(values.color_);
				sprite->SetAnchorPoint(values.anchorPoint_);
				sprite->SetIsFlipX(values.isFlipX_);
				sprite->SetIsFlipY(values.isFlipY_);
			}
		);

		if (!saveRegistered) {
			continue;
		}

#ifdef USE_IMGUI
		const bool editorRegistered = spriteEditor_.RegisterSprite(
			id,
			sprite,
			[sprite]() {
				sprite->ImGuiDebug();
			}
		);

		if (!editorRegistered) {
			spriteSave_.UnregisterSprite(id);
			continue;
		}
#endif

		registeredSprites_.emplace(sprite, id);
	}
}
