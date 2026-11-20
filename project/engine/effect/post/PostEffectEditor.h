#pragma once
#include "PostEffectPreview.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

namespace TKM {
	class DirectXCommon;
	// ゲームのEffectを所有・変更せず、描画時の確認用選択とパラメータだけを管理する。
	class PostEffectEditor {
	public:
		void Initialize(DirectXCommon* dxCommon) { dxCommon_ = dxCommon; }
		void Select(PostEffectPreview effect) {
			if (effect >= PostEffectPreview::Game && effect < PostEffectPreview::Count) { selected_ = static_cast<int>(effect); time_ = 0; }
		}
		PostEffectPreview GetSelection() const { return static_cast<PostEffectPreview>(selected_); }
		void Update(float deltaTime);
		void Finalize();
	private:
		void ImGuiDebug();
		DirectXCommon* dxCommon_ = nullptr;
		int selected_ = 0;
		float time_ = 0.0f;
		Vector2 center_{ 0.5f, 0.5f };
		float rippleDuration_ = 1.0f, rippleAmplitude_ = 0.03f;
		float vignetteIntensity_ = 0.8f, vignetteRadius_ = 0.6f, vignetteSoftness_ = 0.4f;
		Vector4 vignetteColor_{ 0, 0, 0, 1 };
		float auraIntensity_ = 0.5f, auraSize_ = 0.22f;
		Vector3 auraColor_{ 0.2f, 0.6f, 1.0f };
		float fogDensity_ = 0.5f, fogStart_ = 0.17f, fogEnd_ = 0.6f;
		Vector3 fogColor_{ 0.9f, 0.9f, 1.0f };
		float noiseIntensity_ = 0.5f;
		float speedIntensity_ = 1.0f;
		Vector2 speedDirection_{ 1.0f, 0.0f };
		float motionStrength_ = 0.95f;
		bool comicOutline_ = true, comicToon_ = true;
		Vector4 comicLineColor_{ 0, 0, 0, 1 };
		float comicLineWidth_ = 1.0f, comicDepthThreshold_ = 0.015f, comicToonBlend_ = 1.0f;
		int comicShadeSteps_ = 3;
	};
}
