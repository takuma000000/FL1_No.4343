#pragma once

#include <memory>

#include "Enemy.h"
#include "EnemyType.h"
#include "Vector3.h"

namespace TKM {
    class DirectXCommon;
    class Camera;
}

//=============================================================
// 敵ファクトリークラス
// 敵の生成を行うクラス
//=============================================================
class EnemyFactory {
public:
	static std::unique_ptr<Enemy> Create(
		EnemyType type,
		const Vector3& position,
		TKM::DirectXCommon* dxCommon,
		TKM::Camera* camera
	);
};

