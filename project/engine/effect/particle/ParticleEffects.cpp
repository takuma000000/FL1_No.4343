#define NOMINMAX
#include "ParticleEffects.h"
#include "ParticleManager.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <stdexcept>

namespace TKM {
	ParticleEffects* ParticleEffects::GetInstance() { static ParticleEffects instance; return &instance; }
	bool ParticleEffects::Reload(const std::string& name) {
		ParticleEffectSettings settings;
		if (!ParticleEffectStore::Load(name, settings, error_)) { return false; }
		cache_[name] = settings;
		return true;
	}
	bool ParticleEffects::Play(const std::string& name, const Vector3& position) {
		if (!ParticleEffectStore::IsValidName(name)) { error_ = "Invalid effect name"; return false; }
		if (!cache_.contains(name) && !Reload(name)) { return false; }
		return Start("__effect_" + name, cache_.at(name), position);
	}
	bool ParticleEffects::Start(const std::string& group, const ParticleEffectSettings& s, const Vector3& position) {
		try {
			s.Validate();
			if (!std::filesystem::is_regular_file(s.texture)) { throw std::runtime_error("Texture file does not exist"); }
			if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) { throw std::runtime_error("Invalid position"); }
			if (s.loop && emitters_.size() >= kMaxLoopingEffects) { throw std::runtime_error("Too many looping effects (64)"); }
			auto* manager = ParticleManager::GetInstance();
			if (!manager->PrepareEffectGroup(group, s)) { throw std::runtime_error("ParticleManager is not initialized or group limit reached"); }
			manager->EmitEffect(group, s, position);
			if (s.loop) { emitters_.push_back({ group, s, position, 0.0f }); }
			error_.clear(); return true;
		} catch (const std::exception& e) { error_ = e.what(); return false; }
	}
	void ParticleEffects::Stop(const std::string& name) {
		const std::string group = "__effect_" + name;
		std::erase_if(emitters_, [&](const Emitter& e) { return e.group == group; });
		ParticleManager::GetInstance()->ClearGroup(group);
	}
	void ParticleEffects::StopAll() {
		emitters_.clear();
		auto* manager = ParticleManager::GetInstance();
		for (const auto& entry : cache_) { manager->ClearGroup("__effect_" + entry.first); }
		manager->ClearGroup(kPreviewGroup);
	}
	void ParticleEffects::StopPreview() {
		std::erase_if(emitters_, [](const Emitter& e) { return e.group == kPreviewGroup; });
		ParticleManager::GetInstance()->ClearGroup(kPreviewGroup);
	}
	bool ParticleEffects::Preview(const ParticleEffectSettings& settings, const Vector3& position) {
		StopPreview();
		return Start(kPreviewGroup, settings, position);
	}
	void ParticleEffects::Update(float deltaTime) {
		if (!std::isfinite(deltaTime) || deltaTime <= 0.0f) { return; }
		for (auto& e : emitters_) {
			e.elapsed += deltaTime;
			// 長い停止明けに大量発生させない。余剰時間は保持する。
			const int bursts = static_cast<int>(std::min(std::floor(e.elapsed / e.settings.interval), static_cast<float>(kMaxBurstsPerFrame)));
			if (bursts > 0) {
				e.elapsed = std::fmod(e.elapsed, e.settings.interval);
				for (int i = 0; i < bursts; ++i) { ParticleManager::GetInstance()->EmitEffect(e.group, e.settings, e.position); }
			}
		}
	}
}
