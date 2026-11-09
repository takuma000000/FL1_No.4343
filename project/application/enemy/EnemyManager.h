#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <string>

#include "Vector3.h"

class Enemy;
class EnemyDataCatalog;

namespace TKM {
    class Camera;
    class DirectXCommon;
}

//=============================================================
// 敵マネージャークラス
// 敵の生成・更新・描画・破棄を一括管理する。
//=============================================================
class EnemyManager {
public:
    EnemyManager();
    ~EnemyManager();

    EnemyManager(const EnemyManager&) = delete;
    EnemyManager& operator=(const EnemyManager&) = delete;

    /// <summary>
    /// 敵の生成に必要な機能とデータカタログを登録する。
    /// </summary>
    void Initialize(TKM::DirectXCommon* dxCommon, TKM::Camera* camera, const EnemyDataCatalog& dataCatalog);

    /// <summary>
    /// 指定した敵IDのEnemyを生成する。
    /// </summary>
    /// <param name="enemyId">生成する敵の固有ID</param>
    /// <param name="position">敵を生成する座標</param>
    /// <param name="errorMessage">生成に失敗した理由</param>
    /// <returns>生成に成功した場合はtrue</returns>
    bool Spawn(const std::string& enemyId, const Vector3& position, std::string& errorMessage);

    /// <summary>
    /// 所有するすべての敵を更新する。
    /// </summary>
    void Update(float deltaTime, const Vector3& targetPosition);

    /// <summary>
    /// 生存中の敵をすべて描画する。
    /// </summary>
    void Draw();

    /// <summary>
    /// 管理中の敵をすべて破棄する。
    /// </summary>
    void Clear();

    std::size_t GetEnemyCount() const { return enemies_.size(); }
    const std::vector<std::unique_ptr<Enemy>>& GetEnemies() const { return enemies_; }

private:
    // これらはEnemyManagerが所有せず、シーン側が生存期間を管理する。
    TKM::DirectXCommon* dxCommon_ = nullptr;
    TKM::Camera* camera_ = nullptr;
    const EnemyDataCatalog* dataCatalog_ = nullptr;

    // unique_ptrにより、敵の破棄を自動化する。
    std::vector<std::unique_ptr<Enemy>> enemies_;
};

