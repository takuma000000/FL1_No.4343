#define NOMINMAX
#include "ParticleEffects.h"
#include "ParticleManager.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <stdexcept>
#ifdef USE_IMGUI
#include "imgui.h"
#endif

namespace TKM {
	ParticleEffects* ParticleEffects::GetInstance() { static ParticleEffects instance; return &instance; }
	bool ParticleEffects::Reload(const std::string& name) {
		ParticleEffectSettings settings;
		if (!ParticleEffectStore::Load(name, settings, error_)) { return false; }
		cache_[name] = settings;
		dirtyScales_.erase(name);
		ParticleManager::GetInstance()->SetEffectScale("__effect_" + name, settings.scale);
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
			manager->SetEffectScale(group, s.scale);
			manager->EmitEffect(group, s, position);
			if (s.loop) { emitters_.push_back({ group, s, position, 0.0f }); }
			error_.clear(); return true;
		} catch (const std::exception& e) { error_ = e.what(); return false; }
	}
	void ParticleEffects::SetPosition(const std::string& name, const Vector3& position) {
		if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) { return; }
		const std::string group = "__effect_" + name;
		for (auto& emitter : emitters_) {
			if (emitter.group == group) { emitter.position = position; }
		}
	}

	void ParticleEffects::SetScale(const std::string& name, float scale) {
		if (!std::isfinite(scale) || scale < 0.1f || scale > 100.0f) { return; }
		if (!cache_.contains(name) && !Reload(name)) { return; }
		if (cache_.at(name).scale != scale) { dirtyScales_.insert(name); }
		cache_.at(name).scale = scale;
		ParticleManager::GetInstance()->SetEffectScale("__effect_" + name, scale);
	}
	void ParticleEffects::SetSizeMultiplier(const std::string& name, float multiplier) {
		SetScale(name, multiplier);
	}

	void ParticleEffects::SetPreviewScale(float scale) {
		if (std::isfinite(scale) && scale >= 0.1f && scale <= 100.0f) {
			ParticleManager::GetInstance()->SetEffectScale(kPreviewGroup, scale);
		}
	}
	bool ParticleEffects::SaveScale(const std::string& name) {
		if (!cache_.contains(name)) { error_ = "Effect is not loaded"; return false; }
		// ディスク上の最新設定に倍率だけ反映。古いキャッシュで見た目を上書きしない。
		ParticleEffectSettings latest;
		if (!ParticleEffectStore::Load(name, latest, error_)) { return false; }
		latest.scale = cache_.at(name).scale;
		if (!ParticleEffectStore::Save(name, latest, error_)) { return false; }
		dirtyScales_.erase(name);
		return true;
	}
	void ParticleEffects::ImGuiDebug() {
#ifdef USE_IMGUI
		if (cache_.empty()) { return; }
		ImGui::Begin("エフェクトサイズ調整");
		std::vector<std::string> names;
		for (const auto& entry : cache_) { names.push_back(entry.first); }
		std::sort(names.begin(), names.end());
		if (!cache_.contains(selectedScaleEffect_)) { selectedScaleEffect_ = names.front(); }
		if (ImGui::BeginCombo("演出", selectedScaleEffect_.c_str())) {
			for (const auto& name : names) {
				if (ImGui::Selectable(name.c_str(), name == selectedScaleEffect_)) { selectedScaleEffect_ = name; scaleMessage_.clear(); }
			}
			ImGui::EndCombo();
		}
		float scale = cache_.at(selectedScaleEffect_).scale;
		if (ImGui::DragFloat("演出全体の倍率", &scale, 0.1f, 0.1f, 100.0f, "%.1f倍", ImGuiSliderFlags_AlwaysClamp)) {
			SetScale(selectedScaleEffect_, scale);
			scaleMessage_.clear();
		}
		if (ImGui::Button("1倍に戻す")) { SetScale(selectedScaleEffect_, 1.0f); }
		if (dirtyScales_.contains(selectedScaleEffect_)) { ImGui::TextUnformatted("* 倍率は未保存です"); }
		if (ImGui::Button("この演出の倍率を保存")) {
			scaleMessage_ = SaveScale(selectedScaleEffect_) ? "倍率を保存しました" : "保存失敗: " + error_;
		}
		ImGui::SameLine();
		if (ImGui::Button("保存した倍率に戻す")) {
			scaleMessage_ = Reload(selectedScaleEffect_) ? "保存した倍率に戻しました" : "読込失敗: " + error_;
		}
		ImGui::TextWrapped("粒・発生範囲・飛ぶ距離をまとめて拡大します。寿命や発生間隔は同じです。");
		ImGui::TextWrapped("保存先: resources/effects/%s.json（倍率のみ更新）", selectedScaleEffect_.c_str());
		ImGui::TextWrapped("保存した倍率は専用エディタでも使われ、次回のPlayに自動で反映されます。");
		if (!scaleMessage_.empty()) { ImGui::TextWrapped("%s", scaleMessage_.c_str()); }
		ImGui::End();
#endif
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
		// 放出タイマーだけ置き換える。粒子を残すかは共通のEmitEffectで決める。
		std::erase_if(emitters_, [](const Emitter& e) { return e.group == kPreviewGroup; });
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
