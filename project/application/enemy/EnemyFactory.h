#pragma once

#include <memory>
#include <string>

#include "Enemy.h"
#include "EnemyType.h"
#include "Vector3.h"

namespace TKM {
    class DirectXCommon;
    class Camera;
}

class EnemyDataCatalog;

//=============================================================
// 敵ファクトリークラス
// 敵の生成を行うクラス
//=============================================================
class EnemyFactory {
public:
	/// <summary>
	/// Catalogに登録されている敵IDからEnemyを生成する。
	/// </summary>
	static std::unique_ptr<Enemy> Create(
		const std::string& enemyId,
		const EnemyDataCatalog& dataCatalog,
		const Vector3& position,
		TKM::DirectXCommon* dxCommon,
		TKM::Camera* camera,
		std::string& errorMessage
	);

	/// <summary>
	/// EnemyTypeからEnemyを生成する旧方式。
	/// EnemyManagerをID方式へ変更するまで一時的に残す。
	/// </summary>
	static std::unique_ptr<Enemy> Create(
		EnemyType type,
		const Vector3& position,
		TKM::DirectXCommon* dxCommon,
		TKM::Camera* camera
	);
};

