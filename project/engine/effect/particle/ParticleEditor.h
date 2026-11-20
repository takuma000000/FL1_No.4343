#pragma once
#include "ParticleEffectSettings.h"
#include <array>

namespace TKM {
	// 編集UIとファイル操作を担当。描画・粒子シミュレーションは共通ランタイムへ任せる。
	class ParticleEditor {
	public:
		void Initialize();
		void ImGuiDebug();
		bool IsPaused() const { return paused_; }
	private:
		void Restart();
		void Load(const std::string& name);
		void Save();
		void RefreshFiles();
		ParticleEffectSettings settings_;
		Vector3 position_{};
		std::array<char, 65> name_{};
		std::vector<std::string> files_, textures_;
		std::string selected_, message_, pendingLoad_;
		bool playing_ = false, paused_ = false, dirty_ = false;
		bool confirmOverwrite_ = false, confirmNew_ = false;
	};
}
