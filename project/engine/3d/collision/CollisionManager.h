#pragma once
#include "AABBCollider.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace TKM {

	//=============================================================
	// CollisionManagerクラス
	// コライダーの生成・更新・当たり判定を管理するクラス。
	//=============================================================
	class CollisionManager {
	public:
		// オブジェクトの現在位置を取得する処理
		using PositionGetter = std::function<Vector3()>;
		// オブジェクトの現在サイズを取得する処理
		using SizeGetter = std::function<Vector3()>;

		/// <summary>
		/// CollisionManagerのインスタンスを取得します。
		/// </summary>
		static CollisionManager* GetInstance();

		/// <summary>
		/// AABBコライダーを登録します。
		/// </summary>
		/// <param name="id">コライダーの識別名</param>
		/// <param name="positionGetter">現在位置を取得する処理</param>
		/// <param name="sizeGetter">現在サイズを取得する処理</param>
		/// <returns>登録したコライダー</returns>
		AABBCollider* RegisterAABB(
			const std::string& id,
			PositionGetter positionGetter,
			SizeGetter sizeGetter
		);

		/// <summary>
		/// 指定したコライダーを登録解除します。
		/// </summary>
		void Unregister(const std::string& id);

		/// <summary>
		/// 全コライダーの位置更新と当たり判定を行います。
		/// </summary>
		void Update();

		/// <summary>
		/// すべてのコライダーを登録解除します。
		/// </summary>
		void Clear();

		/// <summary>
		/// 登録済みのAABBコライダーを取得します。
		/// </summary>
		AABBCollider* GetAABBCollider(
			const std::string& id
		);

	private:
		CollisionManager() = default;
		~CollisionManager() = default;

		CollisionManager(
			const CollisionManager&
		) = delete;

		CollisionManager& operator=(
			const CollisionManager&
			) = delete;

		//=============================================================
		// 登録情報
		//=============================================================
		struct Entry {
			std::unique_ptr<AABBCollider> collider_;
			PositionGetter positionGetter_;
			SizeGetter sizeGetter_;
		};

		//=============================================================
		// 衝突ペア
		//=============================================================
		struct CollisionPair {
			Collider* first_ = nullptr;
			Collider* second_ = nullptr;

			bool operator<(
				const CollisionPair& other
				) const;
		};

		/// <summary>
		/// 全コライダーの位置とサイズを更新します。
		/// </summary>
		void UpdateColliderTransforms();

		/// <summary>
		/// 全コライダーの当たり判定を行います。
		/// </summary>
		void CheckAllCollisions();

		static CollisionPair MakePair(
			Collider* first,
			Collider* second
		);

		static bool CanCollide(
			const Collider& first,
			const Collider& second
		);

		//======================================================================
		// 登録コライダー
		//======================================================================
		std::map<std::string, Entry> entries_;

		//======================================================================
		// 前フレームの衝突情報
		//======================================================================
		std::set<CollisionPair> previousPairs_;
	};

} // namespace TKM
