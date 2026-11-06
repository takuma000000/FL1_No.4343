#pragma once
#include "MyMath.h"

#include <cstdint>
#include <functional>
#include <string>
#include <utility>

namespace TKM {

	// 前方前言==============================
	class CollisionManager; // CollisionManagerクラス
	//　====================================

	//=============================================================
	// Colliderクラス
	// コライダーの共通情報を管理する基底クラス。
	//=============================================================
	class Collider {
	public:
		virtual ~Collider() = default;

		// 衝突時に実行する処理
		using CollisionCallback = std::function<void(Collider*)>;

		/// <summary>
		/// 別のコライダーと接触しているか判定します。
		/// </summary>
		/// <param name="other">判定対象</param>
		/// <returns>接触している場合はtrue</returns>
		virtual bool IsColliding(
			const Collider& other
		) const = 0;

		/// <summary>
		/// コライダーの中心座標を取得します。
		/// </summary>
		virtual const Vector3& GetCenter() const = 0;

		/// <summary>
		/// コライダーの全体サイズを取得します。
		/// </summary>
		virtual const Vector3& GetSize() const = 0;

		// Setter=====================================
		/// <summary>
		/// コライダーを識別するタグを設定します。
		/// </summary>
		void SetTag(const std::string& tag) {
			tag_ = tag;
		}

		/// <summary>
		/// 所属レイヤーを設定します。
		/// </summary>
		void SetLayer(uint32_t layer) {
			layer_ = layer;
		}

		/// <summary>
		/// 判定対象にするレイヤーを設定します。
		/// </summary>
		void SetMask(uint32_t mask) {
			mask_ = mask;
		}

		/// <summary>
		/// コライダーの有効状態を設定します。
		/// </summary>
		void SetActive(bool active) {
			isActive_ = active;
		}

		/// <summary>
		/// 接触開始時に実行する処理を設定します。
		/// </summary>
		void SetOnCollisionEnter(
			CollisionCallback callback
		) {
			onCollisionEnter_ = std::move(callback);
		}

		/// <summary>
		/// 接触中に実行する処理を設定します。
		/// </summary>
		void SetOnCollisionStay(
			CollisionCallback callback
		) {
			onCollisionStay_ = std::move(callback);
		}

		/// <summary>
		/// 接触終了時に実行する処理を設定します。
		/// </summary>
		void SetOnCollisionExit(
			CollisionCallback callback
		) {
			onCollisionExit_ = std::move(callback);
		}
		// ===========================================

		// Getter=====================================
		/// <summary>
		/// タグを取得します。
		/// </summary>
		const std::string& GetTag() const {
			return tag_;
		}

		/// <summary>
		/// 所属レイヤーを取得します。
		/// </summary>
		uint32_t GetLayer() const {
			return layer_;
		}

		/// <summary>
		/// 判定対象レイヤーを取得します。
		/// </summary>
		uint32_t GetMask() const {
			return mask_;
		}

		/// <summary>
		/// コライダーが有効か取得します。
		/// </summary>
		bool IsActive() const {
			return isActive_;
		}
		// ===========================================

	private:
		//======================================================================
		// 識別情報
		//======================================================================
		std::string tag_; // コライダーを識別するタグ

		//======================================================================
		// レイヤー
		//======================================================================
		uint32_t layer_ = 1u;          // 自身が所属するレイヤー
		uint32_t mask_ = 0xffffffffu; // 判定対象にするレイヤー

		//======================================================================
		// 状態
		//======================================================================
		bool isActive_ = true; // 当たり判定を有効にするか

		//======================================================================
		// 衝突通知
		//======================================================================
		CollisionCallback onCollisionEnter_; // 接触開始
		CollisionCallback onCollisionStay_;  // 接触中
		CollisionCallback onCollisionExit_;  // 接触終了

		//======================================================================
		// 衝突時のコールバック
		//======================================================================
		friend class CollisionManager; // CollisionManagerクラスがアクセスできるようにする

		/// <summary>
		/// 接触開始時のコールバックを呼び出します。
		/// </summary>
		/// <param name="other">接触した相手のコライダー</param>
		void NotifyCollisionEnter(Collider* other) {
			if (onCollisionEnter_) {
				onCollisionEnter_(other);
			}
		}
		/// <summary>
		/// 接触中のコールバックを呼び出します。
		/// </summary>
		/// <param name="other">接触している相手のコライダー</param>
		void NotifyCollisionStay(Collider* other) {
			if (onCollisionStay_) {
				onCollisionStay_(other);
			}
		}
		/// <summary>
		/// 接触終了時のコールバックを呼び出します。
		/// </summary>
		/// <param name="other">接触が終了した相手のコライダー</param>
		void NotifyCollisionExit(Collider* other) {
			if (onCollisionExit_) {
				onCollisionExit_(other);
			}
		}
	};

} // namespace TKM