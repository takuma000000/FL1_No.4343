#include "DeltaTimeManager.h"

#include <algorithm>

namespace TKM {

	DeltaTimeManager* DeltaTimeManager::GetInstance() {
		static DeltaTimeManager instance;
		return &instance;
	}

	void DeltaTimeManager::Initialize() {
		// 現在時刻を前フレーム時刻として保存する
		previousTime_ = std::chrono::steady_clock::now();

		// 初回更新前は60FPS相当の値を使用する
		deltaTime_ = 1.0f / 60.0f;
	}

	void DeltaTimeManager::Update() {
		// 現在時刻を取得する
		const auto currentTime =
			std::chrono::steady_clock::now();

		// 前回更新からの経過時間を計算する
		const std::chrono::duration<float> elapsedTime =
			currentTime - previousTime_;

		// 次回計測のために現在時刻を保存する
		previousTime_ = currentTime;

		// 長時間停止後の極端なDeltaTimeを防ぐ
		constexpr float kMaxDeltaTime = 0.1f;

		deltaTime_ = std::clamp(
			elapsedTime.count(),
			0.0f,
			kMaxDeltaTime
		);
	}

} // namespace TKM