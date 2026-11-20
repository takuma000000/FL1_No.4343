#include "CollisionSettings.h"
#include "CollisionCategory.h"
#include "CollisionManager.h"
#include "Enemy.h"
#include "EnemyManager.h"

#include <cstdint>

using namespace TKM;

void CollisionSettings::Initialize(
	TKM::Object3d* plane,
	Player* player
) {
	// 1. ゲーム内で使用するコライダーを登録する
	RegisterColliders(
		plane,
		player
	);

	// 2. 当たり判定を行う組み合わせを登録する
	RegisterCollisionPairs();
}

void CollisionSettings::RegisterColliders(
	TKM::Object3d* plane,
	Player* player
) {
	// 登録に必要なオブジェクトが未生成の場合は処理しない
	if (!plane || !player) {
		return;
	}

	auto* collisionManager =
		CollisionManager::GetInstance();

	//======================================================================
	// Player
	//======================================================================
	// PlayerのObject3dと種類を登録する。
	// モデルの中心とサイズは頂点情報から自動計算される。
	collisionManager->RegisterAABB(
		"Player",
		player->GetObject3d(),
		CollisionCategory::Player
	);

	//======================================================================
	// Plane
	//======================================================================
	// PlaneのObject3dと種類を登録する。
	// モデルの中心とサイズは頂点情報から自動計算される。
	collisionManager->RegisterAABB(
		"Plane",
		plane,
		CollisionCategory::Plane
	);
}

void CollisionSettings::RegisterCollisionPairs() {
	// CollisionCategory::を省略して書けるようにする
	using enum TKM::CollisionCategory;

	//======================================================================
	// 当たり判定を行う組み合わせ
	//======================================================================
	RegisterPair(Player, Plane); // PlayerとPlane
	RegisterPair(Player, Enemy); // PlayerとEnemy
}

void CollisionSettings::RegisterPair(
	TKM::CollisionCategory first,
	TKM::CollisionCategory second
) {
	CollisionManager::GetInstance()->AddCollisionPair(
		first,
		second
	);
}

void CollisionSettings::SyncEnemyColliders(const EnemyManager& enemyManager) {
	auto* collisionManager = CollisionManager::GetInstance();
	std::set<std::string> activeIds;
	for (const auto& enemy : enemyManager.GetEnemies()) {
		// 敵ごとに識別名を分ける。
		const auto id = "Enemy_" + std::to_string(reinterpret_cast<std::uintptr_t>(enemy->GetObject3d()));
		if (!enemyColliderIds_.contains(id)) {
			//======================================================================
			// Enemy
			//======================================================================
			// EnemyのObject3dと種類を登録する。
			// モデルの中心とサイズは頂点情報から自動計算される。
			auto* collider = collisionManager->RegisterAABB(
				id,
				enemy->GetObject3d(),
				CollisionCategory::Enemy
			);
			if (!collider) {
				continue;
			}
		}
		activeIds.insert(id);
	}

	for (const auto& id : enemyColliderIds_) {
		if (!activeIds.contains(id)) {
			collisionManager->Unregister(id);
		}
	}
	enemyColliderIds_.swap(activeIds);
}

void CollisionSettings::Finalize() {
	for (const auto& id : enemyColliderIds_) {
		CollisionManager::GetInstance()->Unregister(id);
	}
	enemyColliderIds_.clear();

	// Playerのコライダー登録を解除する
	CollisionManager::GetInstance()->Unregister(
		"Player"
	);

	// Planeのコライダー登録を解除する
	CollisionManager::GetInstance()->Unregister(
		"Plane"
	);
}
