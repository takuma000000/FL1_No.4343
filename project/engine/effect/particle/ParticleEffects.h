#pragma once
#include "ParticleEffectSettings.h"
#include <unordered_map>
#include <vector>

namespace TKM {
	// ゲームとエディタ共通の再生窓口。更新はParticleManagerが行う。
	class ParticleEffects {
	public:
		static ParticleEffects* GetInstance();
		// resources/effects/<name>.jsonを初回だけ読み込む。loopならStopまで放出する。
		bool Play(const std::string& name, const Vector3& position);
		bool Reload(const std::string& name);
		// 再生中の同名エフェクトの発生位置を更新する。既に出た粒子は移動させない。
		void SetPosition(const std::string& name, const Vector3& position);
		// 再生開始後に指定。生存中・今後発生する粒の大きさに即時反映（位置・速度は維持）。
		void SetSizeMultiplier(const std::string& name, float multiplier);
		void Stop(const std::string& name);
		void StopAll();
		// エディタの固定スロット。再調整してもGPU用グループを増殖させない。
		bool Preview(const ParticleEffectSettings& settings, const Vector3& position);
		void StopPreview();
		void Update(float deltaTime);
		const std::string& GetLastError() const { return error_; }
	private:
		struct Emitter {
			std::string group;
			ParticleEffectSettings settings;
			Vector3 position;
			float elapsed = 0.0f;
		};
		bool Start(const std::string& group, const ParticleEffectSettings& settings, const Vector3& position);
		std::unordered_map<std::string, ParticleEffectSettings> cache_;
		std::vector<Emitter> emitters_;
		std::string error_;
		static constexpr const char* kPreviewGroup = "__editor_particle_preview";
		static constexpr size_t kMaxLoopingEffects = 64;
		static constexpr int kMaxBurstsPerFrame = 8;
	};
}
