#include "CollisionSettings.h"
#include "CollisionCategory.h"
#include "CollisionManager.h"

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
	auto* collisionManager =
		CollisionManager::GetInstance();

	// PlayerとPlaneの当たり判定を有効にする
	// 逆方向を追加で登録する必要はない。
	collisionManager->AddCollisionPair(
		CollisionCategory::Player,
		CollisionCategory::Plane
	);
}

void CollisionSettings::Finalize() {
	// Playerのコライダー登録を解除する
	CollisionManager::GetInstance()->Unregister(
		"Player"
	);

	// Planeのコライダー登録を解除する
	CollisionManager::GetInstance()->Unregister(
		"Plane"
	);
}
