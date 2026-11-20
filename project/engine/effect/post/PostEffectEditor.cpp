#define NOMINMAX
#include "PostEffectEditor.h"
#include "DirectXCommon.h"
#include <algorithm>
#include <cmath>
#ifdef USE_IMGUI
#include "imgui.h"
#endif

namespace TKM {
	void PostEffectEditor::Finalize() {
		if (dxCommon_) { dxCommon_->SetPostEffectPreview(PostEffectPreview::Game); }
		dxCommon_ = nullptr;
	}
	void PostEffectEditor::ImGuiDebug() {
#ifdef USE_IMGUI
		ImGui::Begin("ポストエフェクト");
		const char* names[] = { "ゲームの設定", "なし", "ラジアルブラー", "波紋", "ビネット", "オーラ（画面効果）", "霧", "ノイズ", "スピード線", "モーションブラー", "漫画調（輪郭線＋トゥーン）" };
		static_assert(IM_ARRAYSIZE(names) == static_cast<int>(PostEffectPreview::Count));
		if (ImGui::Combo("表示する効果", &selected_, names, IM_ARRAYSIZE(names))) { time_ = 0.0f; }
		if (ImGui::Button("ゲームの設定に戻す")) { selected_ = 0; time_ = 0.0f; }
		ImGui::Separator();
		switch (static_cast<PostEffectPreview>(selected_)) {
		case PostEffectPreview::Game: ImGui::TextWrapped("ゲーム側で有効にした効果を表示します。"); break;
		case PostEffectPreview::None: ImGui::TextUnformatted("ポストエフェクトを適用しません。"); break;
		case PostEffectPreview::RadialBlur: ImGui::TextWrapped("画面中央から放射状にぼかします。現在のシェーダーは固定強度です。"); break;
		case PostEffectPreview::Ripple:
			ImGui::SliderFloat2("中心", &center_.x, 0, 1);
			ImGui::SliderFloat("継続時間（秒）", &rippleDuration_, 0.1f, 5.0f);
			ImGui::SliderFloat("ゆがみの強さ", &rippleAmplitude_, 0.0f, 0.15f);
			ImGui::TextUnformatted("波紋を繰り返し再生します。");
			if (ImGui::Button("波紋を最初から")) { time_ = 0.0f; } break;
		case PostEffectPreview::Vignetting:
			ImGui::ColorEdit4("ふちの色", &vignetteColor_.x);
			ImGui::SliderFloat("強さ", &vignetteIntensity_, 0, 2);
			ImGui::SliderFloat("半径", &vignetteRadius_, 0, 1);
			ImGui::SliderFloat("ぼかし幅", &vignetteSoftness_, 0.01f, 1); break;
		case PostEffectPreview::Aura:
			ImGui::SliderFloat2("中心", &center_.x, 0, 1);
			ImGui::SliderFloat("大きさ", &auraSize_, 0.01f, 1);
			ImGui::SliderFloat("明るさ", &auraIntensity_, 0, 5);
			ImGui::ColorEdit3("色", &auraColor_.x); break;
		case PostEffectPreview::Fog:
			ImGui::ColorEdit3("霧の色", &fogColor_.x);
			ImGui::SliderFloat("濃さ", &fogDensity_, 0, 8);
			ImGui::SliderFloat("開始位置", &fogStart_, 0, 0.98f);
			ImGui::SliderFloat("最大位置", &fogEnd_, fogStart_ + 0.01f, 1); break;
		case PostEffectPreview::Noise: ImGui::SliderFloat("強さ", &noiseIntensity_, 0, 2); break;
		case PostEffectPreview::SpeedLine:
			ImGui::SliderFloat("強さ", &speedIntensity_, 0, 2);
			ImGui::SliderFloat2("流れる方向", &speedDirection_.x, -1, 1); break;
		case PostEffectPreview::MotionBlur:
			ImGui::SliderFloat("残像の強さ", &motionStrength_, 0, 0.95f);
			ImGui::TextUnformatted("オブジェクトやカメラを動かすと残像を確認できます。"); break;
		case PostEffectPreview::Comic:
			ImGui::Checkbox("輪郭線", &comicOutline_);
			if (comicOutline_) {
				ImGui::ColorEdit4("線の色", &comicLineColor_.x);
				ImGui::SliderFloat("線の太さ（ピクセル）", &comicLineWidth_, 1, 5, "%.0f");
				ImGui::SliderFloat("境界の判定しきい値", &comicDepthThreshold_, 0.001f, 0.1f, "%.3f");
			}
			ImGui::Checkbox("トゥーンの明暗", &comicToon_);
			if (comicToon_) {
				ImGui::SliderInt("明暗の段階数", &comicShadeSteps_, 2, 8);
				ImGui::SliderFloat("トゥーンの強さ", &comicToonBlend_, 0, 1);
			}
			ImGui::TextWrapped("奥行きの境界に線を描き、ライトの明暗を段階化します。テクスチャの柄やUIには線を付けません。"); break;
		default: break;
		}
		ImGui::Separator();
		ImGui::TextWrapped("選択中の1種類だけ表示します。前面スプライトとImGuiには適用しません。");
		ImGui::TextDisabled("確認用の設定です。ファイルへの自動保存は行いません。");
		ImGui::End();
#endif
	}
	void PostEffectEditor::Update(float deltaTime) {
		if (!dxCommon_) { return; }
		if (std::isfinite(deltaTime) && deltaTime > 0) { time_ += deltaTime; }
		ImGuiDebug();
		const auto effect = static_cast<PostEffectPreview>(selected_);
		dxCommon_->SetPostEffectPreview(effect);
		switch (effect) {
		case PostEffectPreview::RadialBlur: dxCommon_->InitializeRadialBlurPipeline(); break;
		case PostEffectPreview::Ripple: {
			dxCommon_->InitializeWaterRipplePipeline();
			const float t = std::fmod(time_, rippleDuration_) / rippleDuration_;
			dxCommon_->SetWaterRippleParam(center_, t * 0.85f, rippleAmplitude_ * (1-t), 80, 10, { 1, 1, 1 }, 0); break;
		}
		case PostEffectPreview::Vignetting:
			dxCommon_->InitializeVignettingPipeline();
			dxCommon_->SetVignettingParam(vignetteColor_, vignetteIntensity_, vignetteRadius_, vignetteSoftness_); break;
		case PostEffectPreview::Aura:
			dxCommon_->InitializeAuraPipeline();
			dxCommon_->SetAuraParam(center_, {center_.x, center_.y-0.2f}, {center_.x, center_.y+0.2f},
				static_cast<float>(WindowsAPI::GetClientWidth()) / WindowsAPI::GetClientHeight(), time_, auraSize_, auraIntensity_,
				1, 0.12f, 22, auraColor_, { 1, 0.85f, 0.2f }, 0.25f, 0.65f, 6, 1.2f, 1.2f, 2, 0.12f); break;
		case PostEffectPreview::Fog:
			dxCommon_->InitializeFogPipeline();
			fogEnd_ = std::max(fogEnd_, fogStart_ + 0.01f);
			dxCommon_->SetFogParam(fogColor_, fogDensity_, fogStart_, fogEnd_, 10, 0.8f, time_, { 0, 0, 0 }, 0.02f); break;
		case PostEffectPreview::Noise:
			dxCommon_->InitializeNoisePipeline();
			dxCommon_->SetNoiseParam(time_, noiseIntensity_, 180, 8, 32, 0.03f, 0.006f, 0,
				{ static_cast<float>(WindowsAPI::GetClientWidth()), static_cast<float>(WindowsAPI::GetClientHeight()) }); break;
		case PostEffectPreview::SpeedLine: {
			dxCommon_->InitializeSpeedLinePipeline();
			const float length = std::hypot(speedDirection_.x, speedDirection_.y);
			const Vector2 direction = length > 0.001f ? Vector2{ speedDirection_.x / length, speedDirection_.y / length } : Vector2{ 1, 0 };
			dxCommon_->SetSpeedLineParam(direction, speedIntensity_, time_, 42, 18, 0.035f); break;
		}
		case PostEffectPreview::MotionBlur:
			dxCommon_->InitializeMotionBlurPipeline();
			dxCommon_->SetMotionBlurParam(motionStrength_); break;
		case PostEffectPreview::Comic:
			dxCommon_->InitializeComicPipeline();
			dxCommon_->SetComicParam(comicLineColor_, comicLineWidth_, comicDepthThreshold_, comicShadeSteps_,
				comicToonBlend_, comicToon_, comicOutline_ ? 1.0f : 0.0f); break;
		default: break;
		}
	}
}
