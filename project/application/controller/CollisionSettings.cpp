#include "CollisionSettings.h"
#include "CollisionCategory.h"
#include "CollisionManager.h"

using namespace TKM;

//=============================================================================
// 新しいオブジェクトの当たり判定を追加する手順
//=============================================================================
// 1. CollisionCategory.hに、新しいオブジェクトの種類を追加する
//
// 2. RegisterAABB()でコライダーを登録する
//    第1引数：登録名
//    第2引数：現在位置を返す処理
//    第3引数：現在サイズを返す処理
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

	// 1. Playerのコライダーを登録する
	// 位置とサイズはPlayerから毎フレーム自動取得する
	auto* playerCollider =
		CollisionManager::GetInstance()->RegisterAABB(
			"Player",
			[player]() {
				return player->GetPos();
			},
			[player]() {
				return player->GetScale();
			}
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

	// 1. Planeのコライダーを登録する
	// 位置とサイズはObject3dから毎フレーム自動取得する
	auto* planeCollider =
		CollisionManager::GetInstance()->RegisterAABB(
			"Plane",
			[plane]() {
				return plane->GetTranslate();
			},
			[plane]() {
				return plane->GetScale();
			}
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
