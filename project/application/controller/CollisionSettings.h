#pragma once
#include "Object3d.h"
#include "player.h"

//=============================================================
// CollisionSettingsクラス
// ゲーム内オブジェクトのコライダー登録を管理するクラス。
// オブジェクト本体の所有権は持たない。
//=============================================================
class CollisionSettings {
public:
	/// <summary>
	/// ゲーム内オブジェクトのコライダーを登録します。
	/// </summary>
	/// <param name="plane">テスト用Plane</param>
	/// <param name="player">Player</param>
	void Initialize(
		TKM::Object3d* plane,
		Player* player
	);

	/// <summary>
	/// 登録したコライダーを解除します。
	/// </summary>
	void Finalize();

private:
	/// <summary>
	/// ゲーム内で使用するコライダーをまとめて登録します。
	/// </summary>
	void RegisterColliders(
		TKM::Object3d* plane,
		Player* player
	);

	/// <summary>
	/// 当たり判定を行うCategoryの組み合わせを登録します。
	/// </summary>
	void RegisterCollisionPairs();
};
