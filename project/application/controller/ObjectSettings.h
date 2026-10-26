#pragma once
#include "Object3d.h"
#include "ObjectEditor.h"
#include "ObjectSave.h"

#include "Player.h"

//=============================================================
// ObjectSettingsクラス
// ゲームオブジェクトの保存・編集設定を管理するクラス。
// オブジェクト本体の所有権は持たない。
//=============================================================
class ObjectSettings {
public:
	/// <summary>
	/// オブジェクトの保存・編集設定を初期化します。
	/// </summary>
	/// <param name="sphere">テスト用Sphere</param>
	/// <param name="player">Player</param>
	void Initialize(
		TKM::Object3d* sphere,
		Player* player
	);

	/// <summary>
	/// オブジェクトの保存・編集設定を終了します。
	/// </summary>
	void Finalize();

	/// <summary>
	/// ImGuiでオブジェクト編集画面を表示します。
	/// </summary>
	void ImGuiDebug();

private:
	//======================================================================
	// 保存・編集
	//======================================================================
	TKM::ObjectSave objectSave_;     // オブジェクト保存
	TKM::ObjectEditor objectEditor_; // オブジェクト編集
};