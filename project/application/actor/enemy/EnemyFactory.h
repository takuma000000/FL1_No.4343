#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "Enemy.h"
#include "Vector3.h"

namespace TKM {
    class DirectXCommon;
    class Camera;
}

class EnemyDataCatalog;

//=============================================================
// 敵ファクトリークラス
// EnemyDataCatalogに登録されている敵IDからEnemyを生成する。
//=============================================================
class EnemyFactory {
public:
    /// <summary>
    /// Catalogに登録されている敵IDからEnemyを生成する。
    /// </summary>
    /// <param name="instanceId">生成する敵のインスタンスID</param>
    /// <param name="enemyId">生成する敵の種類ID</param>
    /// <param name="dataCatalog">読み込み済みの敵データ</param>
    /// <param name="position">敵を生成する座標</param>
    /// <param name="dxCommon">DirectX共通機能</param>
    /// <param name="camera">Enemyが使用するカメラ</param>
    /// <param name="errorMessage">生成に失敗した理由</param>
    /// <returns>生成したEnemy。失敗時はnullptr</returns>
    static std::unique_ptr<Enemy> Create(
        std::uint64_t instanceId,
        const std::string& enemyId,
        const EnemyDataCatalog& dataCatalog,
        const Vector3& position,
        TKM::DirectXCommon* dxCommon,
        TKM::Camera* camera,
        std::string& errorMessage
    );
};

