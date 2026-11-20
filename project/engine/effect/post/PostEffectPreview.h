#pragma once
namespace TKM {
	// Gameは従来の各Effect::IsActiveに従う。その他はImGui確認用の単独表示。
	enum class PostEffectPreview { Game, None, RadialBlur, Ripple, Vignetting, Aura, Fog, Noise, SpeedLine, MotionBlur, Comic, Count };
	constexpr bool ShouldApplyPostEffect(PostEffectPreview selection, PostEffectPreview effect, bool gameActive) {
		return selection == PostEffectPreview::Game ? gameActive : selection == effect;
	}
}
