#pragma once

#include "Sprite.h"

#include <memory>

namespace TKM {
	class DirectXCommon;

	//=============================================================
	// SceneTransitionクラス
	// シーン切り替え時のフェード表示を管理するクラス。
	//=============================================================
	class SceneTransition {
	public:
		/// <summary>
		/// フェード描画を初期化します。
		/// </summary>
		void Initialize(DirectXCommon* dxCommon);

		/// <summary>
		/// フェード状態を更新します。
		/// </summary>
		void Update();

		/// <summary>
		/// 画面全体へフェードを描画します。
		/// </summary>
		void Draw();

		/// <summary>
		/// 現在のシーンを暗転させます。
		/// </summary>
		void StartFadeOut();

		/// <summary>
		/// 新しいシーンを表示します。
		/// </summary>
		void StartFadeIn();

		/// <summary>
		/// 完全に暗転してシーンを交換できる状態か確認します。
		/// </summary>
		bool IsCovered() const;

		/// <summary>
		/// フェード処理中か確認します。
		/// </summary>
		bool IsActive() const;

		/// <summary>
		/// フェードの片道時間を秒単位で設定します。
		/// </summary>
		void SetDuration(float duration);

	private:
		enum class State {
			Idle,
			FadeOut,
			Covered,
			FadeIn,
		};

		void ApplyAlpha();

		std::unique_ptr<Sprite> fadeSprite_ = nullptr;
		State state_ = State::Idle;
		float alpha_ = 0.0f;
		float duration_ = 0.5f;
	};
}
