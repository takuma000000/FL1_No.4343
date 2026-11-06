#pragma once

#include <memory>

#include "EnemyData.h"
#include "EnemyType.h"

namespace TKM {
	class Camera;
	class DirectXCommon;
	class Object3d;
	class Object3dCommon;
}

//=============================================================
// 敵クラス
// 座標・HP・移動・描画など、敵の共通処理を管理する。
//=============================================================
class Enemy {
public:
	Enemy();
	~Enemy();

	Enemy(const Enemy&) = delete;
	Enemy& operator=(const Enemy&) = delete;

	/// <summary>
	/// 敵を初期化する。
	/// </summary>
	void Initialize(
		EnemyType type,
		const EnemyData& data,
		const Vector3& position,
		TKM::Object3dCommon* object3dCommon,
		TKM::DirectXCommon* dxCommon,
		TKM::Camera* camera
	);

	/// <summary>
	/// ターゲット座標に応じて敵を更新する。
	/// </summary>
	void Update(float deltaTime, const Vector3& targetPosition);

	/// <summary>
	/// 敵の3Dモデルを描画する。
	/// </summary>
	void Draw(TKM::DirectXCommon* dxCommon);

	/// <summary>
	/// ダメージを受け、HPを0未満にならないように更新する。
	/// </summary>
	void TakeDamage(int damage);

	// Getter ====================================================
	EnemyType GetType() const { return type_; }
	const EnemyData& GetData() const { return data_; }
	const Vector3& GetPosition() const { return position_; }
	int GetCurrentHp() const { return currentHp_; }
	float GetCollisionRadius() const { return data_.collisionRadius; }
	bool IsAlive() const { return currentHp_ > 0; }
	bool IsWithinAttackRange(const Vector3& targetPosition) const;

	// Setter ====================================================
	void SetPosition(const Vector3& position) { position_ = position; }

private:
	/// <summary>
	/// 敵の種類に応じた移動を行う。
	/// </summary>
	void UpdateMovement(float deltaTime, const Vector3& targetPosition);

private:
	// 描画対象。Enemyが所有権を持つ。
	std::unique_ptr<TKM::Object3d> object3d_;

	// 敵の種類と調整用パラメータ。
	EnemyType type_ = EnemyType::Normal;
	EnemyData data_{};

	// ゲーム中に変化する値。
	Vector3 position_{ 0.0f, 0.0f, 0.0f };
	int currentHp_ = 0;
};
