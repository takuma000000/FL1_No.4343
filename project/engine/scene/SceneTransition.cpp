#define NOMINMAX

#include "SceneTransition.h"

#include "DeltaTimeManager.h"
#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "WindowsAPI.h"

#include <algorithm>
#include <cassert>

namespace {
	constexpr const char* kFadeTexturePath =
		"./resources/texture/uvChecker.png";
}

namespace TKM {
	void SceneTransition::Initialize(DirectXCommon* dxCommon) {
		assert(dxCommon != nullptr
			&& "dxCommon is nullptr in SceneTransition::Initialize");

		TextureManager::GetInstance()->LoadTexture(kFadeTexturePath);

		fadeSprite_ = std::make_unique<Sprite>();
		fadeSprite_->SetEditorEnabled(false);
		fadeSprite_->Initialize(
			SpriteCommon::GetInstance(),
			dxCommon,
			kFadeTexturePath
		);
		fadeSprite_->SetPosition({ 0.0f, 0.0f });
		fadeSprite_->SetAnchorPoint({ 0.0f, 0.0f });
		fadeSprite_->SetAutoAdjustTextureSize(false);
		fadeSprite_->SetSize({
			static_cast<float>(WindowsAPI::GetClientWidth()),
			static_cast<float>(WindowsAPI::GetClientHeight())
		});

		state_ = State::Idle;
		alpha_ = 0.0f;
		ApplyAlpha();
	}

	void SceneTransition::Update() {
		if (!fadeSprite_ || state_ == State::Idle) {
			return;
		}

		const float deltaTime =
			DeltaTimeManager::GetInstance()->GetDeltaTime();
		const float alphaStep = deltaTime / duration_;

		switch (state_) {
		case State::FadeOut:
			alpha_ = std::min(1.0f, alpha_ + alphaStep);
			if (alpha_ >= 1.0f) {
				state_ = State::Covered;
			}
			break;

		case State::FadeIn:
			alpha_ = std::max(0.0f, alpha_ - alphaStep);
			if (alpha_ <= 0.0f) {
				state_ = State::Idle;
			}
			break;

		case State::Covered:
		case State::Idle:
			break;
		}

		ApplyAlpha();
	}

	void SceneTransition::Draw() {
		if (!fadeSprite_ || alpha_ <= 0.0f) {
			return;
		}

		SpriteCommon::GetInstance()->DrawSetCommon();
		fadeSprite_->Draw();
	}

	void SceneTransition::StartFadeOut() {
		if (!fadeSprite_ || state_ != State::Idle) {
			return;
		}

		alpha_ = 0.0f;
		state_ = State::FadeOut;
		ApplyAlpha();
	}

	void SceneTransition::StartFadeIn() {
		if (!fadeSprite_) {
			return;
		}

		// シーン終了処理でテクスチャ管理が解放されても遷移側で再確保する
		TextureManager::GetInstance()->LoadTexture(kFadeTexturePath);

		alpha_ = 1.0f;
		state_ = State::FadeIn;
		ApplyAlpha();
	}

	bool SceneTransition::IsCovered() const {
		return state_ == State::Covered;
	}

	bool SceneTransition::IsActive() const {
		return state_ != State::Idle;
	}

	void SceneTransition::SetDuration(float duration) {
		duration_ = std::max(0.01f, duration);
	}

	void SceneTransition::ApplyAlpha() {
		if (!fadeSprite_) {
			return;
		}

		fadeSprite_->SetColor({ 0.0f, 0.0f, 0.0f, alpha_ });
		fadeSprite_->Update();
	}
}
