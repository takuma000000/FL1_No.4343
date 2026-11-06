#include "CollisionSettings.h"
#include "CollisionCategory.h"
#include "CollisionManager.h"

using namespace TKM;

//=============================================================================
// 新しいオブジェクトの当たり判定を追加する手順
//=============================================================================
// 1. CollisionCategory.hに、新しいオブジェクトの種類を追加する
//
// 2. RegisterAABB()へ登録名とObject3dを渡す
//    モデルの中心とサイズは自動で計算される
//
// 3. SetCategory()で、このコライダー自身の種類を設定する
//
// 4. AddCollisionTarget()で、当たり判定を取る相手を追加する
//    判定対象が複数ある場合は、AddCollisionTarget()を複数回呼び出す
//
// 5. Finalize()にUnregister()を追加して、終了時に登録を解除する
//=============================================================================

void CollisionSettings::Initialize(
	TKM::Object3d* plane,
	Player* player
) {
	// 登録に必要なオブジェクトが未生成の場合は処理しない
	if (!plane || !player) {
		return;
	}

	//======================================================================
	// Playerの当たり判定設定
	//======================================================================

	// 1. Playerが使用しているObject3dを登録する
	// モデルの中心とサイズは頂点情報から自動計算される
	auto* playerCollider =
		CollisionManager::GetInstance()->RegisterAABB(
			"Player",
			player->GetObject3d()
		);

	// 登録に成功した場合だけ、残りの設定を行う
	if (playerCollider) {
		// 2. 自分の種類をPlayerに設定する
		playerCollider->SetCategory(
			CollisionCategory::Player
		);

		// 3. 当たり判定を取る相手にPlaneを追加する
		playerCollider->AddCollisionTarget(
			CollisionCategory::Plane
		);
	}

	//======================================================================
	// Planeの当たり判定設定
	//======================================================================

	// 1. PlaneのObject3dを登録する
	// モデルの中心とサイズは頂点情報から自動計算される
	auto* planeCollider =
		CollisionManager::GetInstance()->RegisterAABB(
			"Plane",
			plane
		);

	// 登録に成功した場合だけ、残りの設定を行う
	if (planeCollider) {
		// 2. 自分の種類をPlaneに設定する
		planeCollider->SetCategory(
			CollisionCategory::Plane
		);

		// 3. 当たり判定を取る相手にPlayerを追加する
		planeCollider->AddCollisionTarget(
			CollisionCategory::Player
		);
	}
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
