#pragma once

#include <string>

#include "Vector3.h"
#include "EnemyAttribute.h"

//=============================================================
// 敵データ構造体
// 敵のパラメータの定義に使用する
//=============================================================
struct EnemyData {
    //============================================================
	// 基本パラメータ
    //============================================================
	std::string modelName;              // モデル名
	int maxHp = 1;                      // 最大HP
	float moveSpeed = 1.0f;             // 1秒間に移動する距離
	float attackPower = 1.0f;           // 攻撃力
	float attackRange = 2.0f;           // 攻撃を開始する距離
	float collisionRadius = 1.0f;       // 球形の当たり判定に使う半径
	Vector3 scale{ 1.0f, 1.0f, 1.0f };  // モデルの大きさ

    //============================================================
    // 外部データ用の追加パラメータ
    //============================================================
	std::string id;                     // 敵を識別する固有のID
	std::string displayName;            // 画面表示用の名称
	float detectionRange = 0.0f;        // プレイヤーを発見する距離
    float attackInterval = 1.0f;        // 攻撃時の待ち時間（秒）
    int antibodyPoint = 0;              // 撃破時に獲得する抗体ポイント

    //============================================================
    // 属性・弱点・耐性
    //============================================================
    EnemyAttribute attribute = EnemyAttribute::Neutral; // 敵の属性
    EnemyAttribute weakness = EnemyAttribute::None;   // 弱点属性
    EnemyAttribute resistance = EnemyAttribute::None; // 耐性属性
};
