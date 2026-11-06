#pragma once
#include "Collider.h"

#include <set>
#include <vector>

namespace TKM {

	//=============================================================
	// CollisionManagerクラス
	// 登録されたコライダー同士の当たり判定を管理するクラス。
	//=============================================================
	class CollisionManager {
	public:
		/// <summary>
		/// CollisionManagerのインスタンスを取得します。
		/// </summary>
		static CollisionManager* GetInstance();

		/// <summary>
		/// 判定対象のコライダーを登録します。
		/// </summary>
		/// <returns>登録できた場合はtrue</returns>
		bool Register(Collider* collider);

		/// <summary>
		/// コライダーの登録を解除します。
		/// コライダーを破棄する前に呼び出してください。
		/// </summary>
		void Unregister(Collider* collider);

		/// <summary>
		/// 登録されたすべてのコライダーを判定します。
		/// </summary>
		void CheckAllCollisions();

		/// <summary>
		/// すべての登録を解除します。
		/// </summary>
		void Clear();

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
		/// 2つのコライダーから順序が一定のペアを作成します。
		/// </summary>
		static CollisionPair MakePair(
			Collider* first,
			Collider* second
		);

		/// <summary>
		/// レイヤー設定上、判定対象になるか確認します。
		/// </summary>
		static bool CanCollide(
			const Collider& first,
			const Collider& second
		);

		//======================================================================
		// 登録情報
		//======================================================================
		std::vector<Collider*> colliders_;

		//======================================================================
		// 前フレームの衝突情報
		//======================================================================
		std::set<CollisionPair> previousPairs_;
	};

} // namespace TKM