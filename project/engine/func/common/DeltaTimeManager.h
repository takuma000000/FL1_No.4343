#pragma once
#include <chrono>

namespace TKM {

	//=============================================================
	// DeltaTimeManagerクラス
	// フレーム間の経過時間を計測・管理するクラス。
	//=============================================================
	class DeltaTimeManager {
	public:
		/// <summary>
		/// DeltaTimeManagerのインスタンスを取得します。
		/// </summary>
		static DeltaTimeManager* GetInstance();

		/// <summary>
		/// DeltaTimeの計測を初期化します。
		/// </summary>
		void Initialize();

		/// <summary>
		/// DeltaTimeを更新します。
		/// 1フレームに1回だけ呼び出してください。
		/// </summary>
		void Update();

		// Getter=====================================
		/// <summary>
		/// 前フレームからの経過時間を秒単位で取得します。
		/// </summary>
		float GetDeltaTime() const {
			return deltaTime_;
		}

		/// <summary>
		/// DeltaTimeから現在のFPSを取得します。
		/// </summary>
		float GetFPS() const {
			if (deltaTime_ <= 0.0f) {
				return 0.0f;
			}

			return 1.0f / deltaTime_;
		}
		// ===========================================

	private:
		DeltaTimeManager() = default;
		~DeltaTimeManager() = default;

		DeltaTimeManager(const DeltaTimeManager&) = delete;
		DeltaTimeManager& operator=(
			const DeltaTimeManager&
			) = delete;

		//======================================================================
		// DeltaTime
		//======================================================================
		std::chrono::steady_clock::time_point previousTime_;
		float deltaTime_ = 1.0f / 60.0f;
	};

} // namespace TKM