#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "EnemyType.h"
#include "Vector3.h"

class Enemy;

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
	/// 敵の生成に必要なエンジン機能を登録する。
	/// </summary>
	void Initialize(TKM::DirectXCommon* dxCommon, TKM::Camera* camera);

	/// <summary>
	/// 指定した種類の敵を生成する。
	/// </summary>
	void Spawn(EnemyType type, const Vector3& position);

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

	// unique_ptrにより、敵の破棄を自動化する。
	std::vector<std::unique_ptr<Enemy>> enemies_;
};

