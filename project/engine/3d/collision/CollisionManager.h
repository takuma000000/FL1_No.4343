#pragma once
#include "AABBCollider.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace TKM {
	class Object3d;

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
		/// Object3dのモデル範囲からAABBコライダーを自動生成します。
		/// 位置・回転・Scaleの変更も毎フレーム反映されます。
		/// </summary>
		/// <param name="id">コライダーの識別名</param>
		/// <param name="object">判定対象の3Dオブジェクト</param>
		/// <returns>登録したコライダー</returns>
		AABBCollider* RegisterAABB(
			const std::string& id,
			const Object3d* object
		);

		/// <summary>
		/// Object3dと自身の種類をまとめて登録します。
		/// </summary>
		AABBCollider* RegisterAABB(
			const std::string& id,
			const Object3d* object,
			CollisionCategory category
		);

		/// <summary>
		/// 当たり判定を行う2種類の組み合わせを登録します。
		/// AとBの順番を入れ替えて登録する必要はありません。
		/// </summary>
		void AddCollisionPair(
			CollisionCategory first,
			CollisionCategory second
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
		/// 登録されているAABBコライダーをラインで表示します。
		/// 通常時は緑、接触中は赤で表示します。
		/// </summary>
		/// <param name="viewProjection">描画に使用するビュー射影行列</param>
		void DrawDebug(
			const Matrix4x4& viewProjection
		);

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

		using CategoryPair =
			std::pair<uint32_t, uint32_t>;

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

		bool CanCollide(
			const Collider& first,
			const Collider& second
		) const;

		//======================================================================
		// 登録コライダー
		//======================================================================
		std::map<std::string, Entry> entries_;

		//======================================================================
		// 当たり判定を行うCategoryの組み合わせ
		//======================================================================
		std::set<CategoryPair> categoryPairs_;

		//======================================================================
		// 前フレームの衝突情報
		//======================================================================
		std::set<CollisionPair> previousPairs_;

		//======================================================================
		// 現在接触しているコライダー
		//======================================================================
		// デバッグ表示で、接触中のAABBを赤色にするために使用する。
		std::set<Collider*> collidingColliders_;
	};

} // namespace TKM
