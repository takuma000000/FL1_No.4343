#pragma once
#include "Collider.h"
#include "AABB.h"

namespace TKM {

	//=============================================================
	// AABBColliderクラス
	// AABBによる当たり判定を管理するクラス。
	//=============================================================
	class AABBCollider : public Collider {
	public:
		AABBCollider();

		AABBCollider(
			const Vector3& center,
			const Vector3& size
		);

		/// <summary>
		/// 中心座標とサイズを設定します。
		/// </summary>
		void Set(
			const Vector3& center,
			const Vector3& size
		);

		// Setter=====================================
		/// <summary>
		/// 中心座標を設定します。
		/// </summary>
		void SetCenter(const Vector3& center);

		/// <summary>
		/// 全体サイズを設定します。
		/// </summary>
		void SetSize(const Vector3& size);
		// ===========================================

		// Collider===================================
		/// <summary>
		/// 別のコライダーと接触しているか判定します。
		/// </summary>
		bool IsColliding(
			const Collider& other
		) const override;

		/// <summary>
		/// 中心座標を取得します。
		/// </summary>
		const Vector3& GetCenter() const override {
			return center_;
		}

		/// <summary>
		/// 全体サイズを取得します。
		/// </summary>
		const Vector3& GetSize() const override {
			return size_;
		}
		// ===========================================

	private:
		/// <summary>
		/// AABBへ現在の中心座標とサイズを反映します。
		/// </summary>
		void UpdateBounds();

		//======================================================================
		// AABB
		//======================================================================
		Vector3 center_{};                  // 中心座標
		Vector3 size_{ 1.0f, 1.0f, 1.0f }; // 全体サイズ
		AABB bounds_;                       // 当たり判定本体
	};

} // namespace TKM