#pragma once

#include <string>
#include "Vector3.h"

//=============================================================
// 敵データ構造体
// 敵のパラメータの定義に使用する
//=============================================================
struct EnemyData {
	std::string modelName;              // モデル名
	int maxHp = 1;                      // 最大HP
	float moveSpeed = 1.0f;             // 1秒間に移動する距離
	float attackPower = 1.0f;           // 攻撃力
	float attackRange = 2.0f;           // 攻撃を開始する距離
	float collisionRadius = 1.0f;       // 球形の当たり判定に使う半径
	Vector3 scale{ 1.0f, 1.0f, 1.0f };  // モデルの大きさ
};
