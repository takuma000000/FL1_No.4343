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

void EnemyManager::Initialize(TKM::DirectXCommon* dxCommon, TKM::Camera* camera, const EnemyDataCatalog& dataCatalog) {
    assert(dxCommon != nullptr && "EnemyManager::Initialize: dxCommon is null");
    assert(camera != nullptr && "EnemyManager::Initialize: camera is null");

    dxCommon_ = dxCommon;
    camera_ = camera;
    dataCatalog_ = &dataCatalog;

    // シーン再初期化時に、前回の敵が残らないようにする。
    Clear();
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

	std::unique_ptr<Enemy> enemy =
		EnemyFactory::Create(
			enemyId,
			*dataCatalog_,
			position,
			dxCommon_,
			camera_,
			errorMessage
		);

	if (!enemy) {
		return false;
	}

	enemies_.push_back(
		std::move(enemy)
	);

	return true;
}

void EnemyManager::Update(
    float deltaTime,
    const Vector3& targetPosition) {

    // 前フレームの攻撃情報を残さないように初期化する。
    attackEvents_.clear();

    // 管理中の敵を更新し、発生した攻撃を収集する。
    for (const auto& enemy : enemies_) {

        enemy->Update(deltaTime, targetPosition);

        // このフレームに攻撃していない敵は無視する。
        if (!enemy->DidAttackThisFrame()) {
            continue;
        }

        // 外部でダメージ処理などを行えるように、
        // 攻撃した敵の情報を値として保存する。
        EnemyAttackEvent attackEvent{};
        attackEvent.enemyId = enemy->GetData().id;
        attackEvent.enemyPosition = enemy->GetPosition();
        attackEvent.damage = enemy->GetData().attackPower;

        attackEvents_.push_back(
            std::move(attackEvent)
        );
    }

    // HPが0になった敵を更新後にまとめて破棄する。
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
    enemies_.clear();
    attackEvents_.clear();
}
