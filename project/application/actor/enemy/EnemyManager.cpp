#include "EnemyManager.h"

#include <algorithm>
#include <cassert>
#include <utility>

#include "Enemy.h"
#include "EnemyFactory.h"
#include "data/EnemyDataCatalog.h"

// Enemyの完全な型が見えているこのファイルで、
// コンストラクターとデストラクターを定義する。
EnemyManager::EnemyManager() = default;
EnemyManager::~EnemyManager() = default;

bool EnemyManager::Initialize(
    TKM::DirectXCommon* dxCommon,
    TKM::Camera* camera,
    const std::filesystem::path& dataDirectory,
    std::string& errorMessage
) {
    assert(dxCommon != nullptr && "EnemyManager::Initialize: dxCommon is null");
    assert(camera != nullptr && "EnemyManager::Initialize: camera is null");

    errorMessage.clear();
    if (!dxCommon || !camera) {
        errorMessage = "EnemyManagerの初期化に必要な機能が設定されていません。";
        return false;
    }

    auto dataCatalog = std::make_unique<EnemyDataCatalog>();
    if (!dataCatalog->LoadDirectory(dataDirectory, errorMessage)) {
        return false;
    }

    dxCommon_ = dxCommon;
    camera_ = camera;
    dataCatalog_ = std::move(dataCatalog);

    // 再初期化時に、以前の敵や未処理イベントを残さない。
    Clear();

    return true;
}

void EnemyManager::SetCamera(TKM::Camera* camera) {
    if (!camera) {
        return;
    }

    camera_ = camera;

    // カメラ切り替えを、すでに生成済みの敵にも反映する。
    for (const auto& enemy : enemies_) {
        enemy->SetCamera(camera_);
    }
}

bool EnemyManager::Spawn(
    const std::string& enemyId,
    const Vector3& position,
    std::string& errorMessage
) {
    // 前回のエラー内容が残らないように初期化する。
    errorMessage.clear();

    // Releaseビルドではassertが無効になるため、
    // 実際のポインタ確認も行う。
    if (!dxCommon_) {
        errorMessage =
            "EnemyManagerが初期化されていません: dxCommon is null";

        return false;
    }

    if (!camera_) {
        errorMessage =
            "EnemyManagerが初期化されていません: camera is null";

        return false;
    }

    if (!dataCatalog_) {
        errorMessage =
            "EnemyManagerが初期化されていません: dataCatalog is null";

        return false;
    }

    // IDは生成成功時だけ消費する。生成失敗時には同じIDを再利用する。
    std::unique_ptr<Enemy> enemy =
        EnemyFactory::Create(
            nextInstanceId_,
            enemyId,
            *dataCatalog_,
            position,
            dxCommon_,
            camera_,
            errorMessage
        );

    // 生成に失敗した場合は、エラー内容を呼び出し側へ返す。
    if (!enemy) {
        return false;
    }

    // 生成に成功したため、次の個体用IDへ進める。
    ++nextInstanceId_;

    // 生成に成功したEnemyを管理下に置く。
    enemies_.push_back(
        std::move(enemy)
    );

    return true;
}

void EnemyManager::Update(
    float deltaTime,
    const Vector3& targetPosition
) {

    // 前フレームの攻撃情報を残さないように初期化する。
    attackEvents_.clear();

    //=============================================================
    // 敵の更新と攻撃イベントの収集
    //=============================================================
    for (const auto& enemy : enemies_) {
        enemy->Update(deltaTime, targetPosition);

        // このフレームに攻撃していない敵は無視する。
        if (!enemy->DidAttackThisFrame()) {
            continue;
        }

        // Enemyへの参照を残すと、敵削除後に無効になる。
        // そのため、外部処理に必要な情報は値として保存する。
        EnemyAttackEvent attackEvent{};
        attackEvent.enemyInstanceId = enemy->GetInstanceId();
        attackEvent.enemyId = enemy->GetData().id;
        attackEvent.enemyPosition = enemy->GetPosition();
        attackEvent.damage = enemy->GetData().attackPower;

        attackEvents_.push_back(
            std::move(attackEvent)
        );
    }

    //=============================================================
    // 死亡した敵の破棄
    //=============================================================

    // HPが0になった敵を、全Enemyの更新後にまとめて破棄する。
    // イテレート中にeraseしないことで、参照の無効化を防ぐ。
    std::erase_if(
        enemies_,
        [](const std::unique_ptr<Enemy>& enemy) {
            return !enemy->IsAlive();
        }
    );
}

void EnemyManager::Draw() {
    if (!dxCommon_) {
        return;
    }

    for (const auto& enemy : enemies_) {
        enemy->Draw(dxCommon_);
    }
}

void EnemyManager::Clear() {
    // 管理中の敵と、未処理の攻撃イベントを破棄する。
    // nextInstanceId_は、過去の個体とのID重複を防ぐため維持する。
    enemies_.clear();
    attackEvents_.clear();
}
