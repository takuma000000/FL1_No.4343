#include "Enemy.h"

#include <algorithm>
#include <cmath>

#include "Camera.h"
#include "DirectXCommon.h"
#include "MyMath.h"
#include "Object3d.h"
#include "Object3dCommon.h"

// Object3dの完全な型が見えているこのファイルで、
// コンストラクターとデストラクターを定義する。
Enemy::Enemy() = default;
Enemy::~Enemy() = default;

void Enemy::Initialize(
    EnemyType type,
    const EnemyData& data,
    const Vector3& position,
    TKM::Object3dCommon* object3dCommon,
    TKM::DirectXCommon* dxCommon,
    TKM::Camera* camera
) {
    type_ = type;
    data_ = data;
    position_ = position;
    currentHp_ = data_.maxHp;

    // 生成直後は攻撃できる状態にする。
    attackCooldownTimer_ = 0.0f;

    didAttackThisFrame_ = false;

    // Object3dは描画だけを担当し、ゲームの値はEnemy側で管理する。
    object3d_ = std::make_unique<TKM::Object3d>();
    object3d_->Initialize(object3dCommon, dxCommon);
    object3d_->SetModel(data_.modelName);
    object3d_->SetScale(data_.scale);
    object3d_->SetTranslate(position_);
    object3d_->SetCamera(camera);
    object3d_->Update();
}

void Enemy::Update(float deltaTime, const Vector3& targetPosition) {
    // 攻撃を行ったかどうかのフラグをリセットする。
    didAttackThisFrame_ = false;

    // 死亡後は移動やObject3dの更新を行わない。
    if (!IsAlive() || !object3d_) {
        return;
    }

    // 次の攻撃までの待機時間を更新する
    UpdateAttackCooldown(deltaTime);

    // ターゲットが索敵範囲内にいる場合だけ追跡する。
    if (IsWithinDetectionRange(targetPosition)) {
        // 攻撃可能な状態であれば、攻撃を開始する。
        TryAttack(targetPosition);

        // 攻撃可能な距離まで接近していない場合は、移動する。
        UpdateMovement(deltaTime, targetPosition);
    }

    // 移動しなかった場合も描画情報は更新する。
    object3d_->SetTranslate(position_);
    object3d_->Update();
}

void Enemy::Draw(TKM::DirectXCommon* dxCommon) {
    if (!IsAlive() || !object3d_) {
        return;
    }

    object3d_->Draw(dxCommon);
}

void Enemy::SetCamera(TKM::Camera* camera) {
    if (object3d_) {
        object3d_->SetCamera(camera);
    }
}

void Enemy::TakeDamage(int damage) {
    // 0以下のダメージや、死亡後のダメージは無視する。
    if (damage <= 0 || !IsAlive()) {
        return;
    }

    currentHp_ = (std::max)(0, currentHp_ - damage);
}

bool Enemy::IsWithinAttackRange(const Vector3& targetPosition) const {
    Vector3 difference = targetPosition - position_;

    // 地上敵はXZ平面上の距離で判定する。
    if (type_ != EnemyType::Flying) {
        difference.y = 0.0f;
    }

    const float attackRangeSquared =
        data_.attackRange * data_.attackRange;

    return difference.lengthSquared() <= attackRangeSquared;
}

void Enemy::UpdateMovement(
    float deltaTime,
    const Vector3& targetPosition
) {
    Vector3 difference = targetPosition - position_;

    // 地上敵はXZ平面上だけを移動する。
    if (type_ != EnemyType::Flying) {
        difference.y = 0.0f;
    }

    const float distance = std::sqrt(
        difference.lengthSquared()
    );

    // すでに攻撃範囲内なら移動しない。
    if (distance <= data_.attackRange) {
        return;
    }

    const Vector3 direction = MyMath::SafeNormalize(
        difference,
        { 0.0f, 0.0f, 0.0f }
    );

    // 攻撃範囲へ到達するまでの残り距離。
    const float remainingDistance =
        distance - data_.attackRange;

    // 1フレームの移動量を残り距離以下に制限する。
    const float requestedMoveDistance =
        data_.moveSpeed * (std::max)(0.0f, deltaTime);

    const float actualMoveDistance = (std::min)(
        requestedMoveDistance,
        remainingDistance
        );

    position_ += direction * actualMoveDistance;
}

bool Enemy::IsWithinDetectionRange(const Vector3& targetPosition) const {
    Vector3 difference = targetPosition - position_;

    // 地上敵はXZ平面上の距離で判定する。
    if (type_ != EnemyType::Flying) {
        difference.y = 0.0f;
    }

    const float detectionRangeSquared =
        data_.detectionRange * data_.detectionRange;

    return difference.lengthSquared() <= detectionRangeSquared;
}

void Enemy::UpdateAttackCooldown(float deltaTime) {

    // すでに攻撃可能な場合は更新しない。
    if (attackCooldownTimer_ <= 0.0f) {
        return;
    }

    // 経過時間を差し引き、負の値にならないようにする。
    attackCooldownTimer_ = (std::max)(
        0.0f,
        attackCooldownTimer_ - deltaTime
        );
}

void Enemy::TryAttack(const Vector3& targetPosition) {

    // 攻撃範囲外では攻撃しない。
    if (!IsWithinAttackRange(targetPosition)) {
        return;
    }

    // クールタイム中は攻撃しない。
    if (attackCooldownTimer_ > 0.0f) {
        return;
    }

    // このフレームに攻撃が発生したことを記録する。
    didAttackThisFrame_ = true;

    // JSONから読み込んだ攻撃間隔を設定し、
    // 次の攻撃まで待機させる。
    attackCooldownTimer_ = data_.attackInterval;
}
