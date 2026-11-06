#pragma once
#include "MyMath.h"

#include <cstdint>
#include <string>

namespace TKM {

	//=============================================================
	// Colliderクラス
	// コライダーの共通情報を管理する基底クラス。
	//=============================================================
	class Collider {
	public:
		virtual ~Collider() = default;

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
	};

} // namespace TKM