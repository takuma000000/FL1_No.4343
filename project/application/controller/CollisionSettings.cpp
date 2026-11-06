#include "CollisionSettings.h"
#include "CollisionCategory.h"
#include "CollisionManager.h"

using namespace TKM;

void CollisionSettings::Initialize(
	TKM::Object3d* plane,
	Player* player
) {
	// 1. 登録に必要なオブジェクトが存在するか確認する
	// どちらかが未生成の場合は、安全のため登録処理を行わない。
	if (!plane || !player) {
		return;
	}

	//======================================================================
	// Playerの当たり判定設定
	//======================================================================

	// 1. PlayerのAABBコライダーを作成して登録する
	//
	// 第1引数：
	//   コライダーを識別する名前
	//
	// 第2引数：
	//   Playerの現在位置を取得する処理
	//
	// 第3引数：
	//   Playerの現在サイズを取得する処理
	//
	// 位置とサイズは毎フレーム自動で取得されるため、
	// Player側の数値を変更するとコライダーにも反映される。
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

	// コライダーの登録に成功した場合だけ、種類と判定対象を設定する
	if (playerCollider) {
		// 2. このコライダー自身がPlayerであることを設定する
		playerCollider->SetCategory(
			CollisionCategory::Player
		);

		// 3. Playerが当たり判定を取る相手を設定する
		// 今回はPlaneだけを判定対象にする
		playerCollider->AddCollisionTarget(
			CollisionCategory::Plane
		);
	}

	//======================================================================
	// Planeの当たり判定設定
	//======================================================================

	// 1. PlaneのAABBコライダーを作成して登録する
	//
	// PlaneもObject3dが持つ現在位置と現在スケールを使用するため、
	// ImGuiなどで変更した数値がコライダーへ自動で反映される。
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

	// コライダーの登録に成功した場合だけ、種類と判定対象を設定する
	if (planeCollider) {
		// 2. このコライダー自身がPlaneであることを設定する
		planeCollider->SetCategory(
			CollisionCategory::Plane
		);

		// 3. Planeが当たり判定を取る相手を設定する
		// 今回はPlayerだけを判定対象にする
		planeCollider->AddCollisionTarget(
			CollisionCategory::Player
		);
	}
}

void CollisionSettings::Finalize() {
	// 1. Playerのコライダー登録を解除する
	CollisionManager::GetInstance()->Unregister(
		"Player"
	);

	// 2. Planeのコライダー登録を解除する
	CollisionManager::GetInstance()->Unregister(
		"Plane"
	);
}
