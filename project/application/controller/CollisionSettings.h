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
};
