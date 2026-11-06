#include "EnemyManager.h"

#include <algorithm>
#include <cassert>

#include "Enemy.h"
#include "EnemyFactory.h"

// Enemyの完全な型が見えているこのファイルで、
// コンストラクターとデストラクターを定義する。
EnemyManager::EnemyManager() = default;
EnemyManager::~EnemyManager() = default;

void EnemyManager::Initialize(
	TKM::DirectXCommon* dxCommon,
	TKM::Camera* camera
) {
	assert(dxCommon != nullptr && "EnemyManager::Initialize: dxCommon is null");
	assert(camera != nullptr && "EnemyManager::Initialize: camera is null");

	dxCommon_ = dxCommon;
	camera_ = camera;

	// シーン再初期化時に、前回の敵が残らないようにする。
	Clear();
}

void EnemyManager::Spawn(EnemyType type, const Vector3& position) {
	assert(dxCommon_ != nullptr && "EnemyManager::Spawn: manager is not initialized");
	assert(camera_ != nullptr && "EnemyManager::Spawn: camera is null");

	enemies_.push_back(
		EnemyFactory::Create(type, position, dxCommon_, camera_)
	);
}

void EnemyManager::Update(float deltaTime, const Vector3& targetPosition) {
	for (const auto& enemy : enemies_) {
		enemy->Update(deltaTime, targetPosition);
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
	enemies_.clear();
}
