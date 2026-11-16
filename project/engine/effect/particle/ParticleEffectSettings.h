#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cstdint>
#include <string>
#include <vector>

namespace TKM {
	// エディタとゲームで共有する保存データ。プレビューのカメラ・位置は含めない。
	struct ParticleEffectSettings {
		std::string texture = "resources/texture/circle.png";
		int shape = 0; // NORMAL / RING / CYLINDER / RIBBON
		uint32_t count = 24;
		bool loop = false;
		bool keepParticles = true; // 次の発生時も生存粒子を寿命まで残す
		float interval = 0.15f;
		float scale = 1.0f; // 粒・発生範囲・軌跡に共通の保存倍率
		Vector3 spawnExtent = { 0.3f, 0.3f, 0.3f };
		Vector3 velocityMin = { -2.0f, 1.0f, -2.0f };
		Vector3 velocityMax = { 2.0f, 4.0f, 2.0f };
		Vector3 gravity = { 0.0f, -2.0f, 0.0f };
		float lifeMin = 0.6f;
		float lifeMax = 1.2f;
		float startSize = 0.5f;
		float endSize = 0.0f;
		Vector4 startColor = { 1.0f, 0.5f, 0.1f, 1.0f };
		Vector4 endColor = { 1.0f, 0.1f, 0.0f, 0.0f };
		void Validate() const; // 不正な値は例外。ファイルやGPUには触れない。
	};

	class ParticleEffectStore {
	public:
		static constexpr const char* kDirectory = "resources/effects";
		static bool IsValidName(const std::string& name);
		static bool Exists(const std::string& name);
		static std::vector<std::string> List();
		// 失敗時はoutと既存ファイルを変更しない。errorに理由を返す。
		static bool Load(const std::string& name, ParticleEffectSettings& out, std::string& error);
		static bool Save(const std::string& name, const ParticleEffectSettings& settings, std::string& error);
	};
}
