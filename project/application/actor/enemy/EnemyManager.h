#pragma once

#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "Vector3.h"

class Enemy;
class EnemyDataCatalog;

namespace TKM {
    class Camera;
    class DirectXCommon;
}

struct EnemyAttackEvent {
    // 同じ種類の敵が複数いても区別できる、生成個体ごとのID。
    std::uint64_t enemyInstanceId = 0;

    // JSONに定義された敵の種類ID（例: BasicVirus）。
    std::string enemyId;

    // 攻撃が発生したときの敵の座標。
    Vector3 enemyPosition{ 0.0f, 0.0f, 0.0f };

    // ターゲットへ与えるダメージ量。
    float damage = 0.0f;
};

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
    /// 敵の生成に必要な機能を登録し、敵データを読み込む。
    /// </summary>
    bool Initialize(
        TKM::DirectXCommon* dxCommon,
        TKM::Camera* camera,
        const std::filesystem::path& dataDirectory,
        std::string& errorMessage
    );

    /// <summary>
    /// 管理中の敵へ描画用カメラを設定する。
    /// </summary>
    void SetCamera(TKM::Camera* camera);

    /// <summary>
    /// 指定した敵IDのEnemyを生成する。
    /// </summary>
    /// <param name="enemyId">生成する敵の固有ID</param>
    /// <param name="position">敵を生成する座標</param>
    /// <param name="errorMessage">生成に失敗した理由</param>
    /// <returns>生成に成功した場合はtrue</returns>
    bool Spawn(
        const std::string& enemyId,
        const Vector3& position,
        std::string& errorMessage
    );

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

    //=============================================================
    // Getter
    //=============================================================

    /// <summary>
    /// 現在管理している敵の数を取得する。
    /// </summary>
    /// <returns>管理中の敵の数</returns>
    std::size_t GetEnemyCount() const { return enemies_.size(); }

    /// <summary>
    /// 現在管理している敵の一覧を取得する。
    /// </summary>
    /// <returns>管理中の敵の一覧</returns>
    const std::vector<std::unique_ptr<Enemy>>& GetEnemies() const { return enemies_; }

    /// <summary>
    /// このフレームに発生した敵の攻撃イベントを取得する。
    /// </summary>
    /// <returns>敵の攻撃イベント一覧</returns>
    const std::vector<EnemyAttackEvent>& GetAttackEvents() const { return attackEvents_; }

private:
    // DirectXCommonとCameraは所有せず、シーン側が生存期間を管理する。
    TKM::DirectXCommon* dxCommon_ = nullptr;
    TKM::Camera* camera_ = nullptr;

    // EnemyManagerが敵データの読み込み結果を所有する。
    std::unique_ptr<EnemyDataCatalog> dataCatalog_;

    // unique_ptrにより、敵の破棄を自動化する。
    std::vector<std::unique_ptr<Enemy>> enemies_;

    // このフレームに発生した攻撃イベント。
    // Updateの開始時に毎フレーム空にする。
    std::vector<EnemyAttackEvent> attackEvents_;

    // 次に生成する敵へ割り当てるID。
    // Clear後も過去の個体と重複しないよう、Clearではリセットしない。
    std::uint64_t nextInstanceId_ = 1;
};
