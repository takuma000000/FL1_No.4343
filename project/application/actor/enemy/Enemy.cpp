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
    std::uint64_t instanceId,
    EnemyType type,
    const EnemyData& data,
    const Vector3& position,
    TKM::Object3dCommon* object3dCommon,
    TKM::DirectXCommon* dxCommon,
    TKM::Camera* camera
) {
    // 識別情報と、敵データから決まる初期状態を保存する。
    instanceId_ = instanceId;
    type_ = type;
    data_ = data;
    position_ = position;
    currentHp_ = data_.maxHp;

    // 生成直後は攻撃できる状態にする。
    attackCooldownTimer_ = 0.0f;

    didAttackThisFrame_ = false;

    // Object3dは描画だけを担当し、座標やHPなどのゲーム状態は
    // Enemy側で管理する。
    object3d_ = std::make_unique<TKM::Object3d>();
    object3d_->Initialize(object3dCommon, dxCommon);
    object3d_->SetModel(data_.modelName);
    object3d_->SetScale(data_.scale);
    object3d_->SetTranslate(position_);
    object3d_->SetCamera(camera);
    object3d_->Update();
}

void Enemy::Update(float deltaTime, const Vector3& targetPosition) {
    // 前フレームの攻撃結果を残さない。
    // 死亡済みでも必ずリセットできるよう、生存判定より先に行う。
    didAttackThisFrame_ = false;

    // 死亡後は移動やObject3dの更新を行わない。
    if (!IsAlive() || !object3d_) {
        return;
    }

    // 次の攻撃までの待機時間を更新する。
    UpdateAttackCooldown(deltaTime);

    // ターゲットが索敵範囲内にいる場合だけ追跡する。
    if (IsWithinDetectionRange(targetPosition)) {
        // 攻撃範囲とクールタイムの条件を満たせば攻撃する。
        TryAttack(targetPosition);

        // まだ攻撃範囲外なら、範囲の境界まで接近する。
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

    // 地上敵は高さを追跡しないためY軸を除外し、
    // FlyingのみXYZの3次元距離を使用する。
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

    // 距離判定と同じ基準になるよう、地上敵はY軸方向へ移動しない。
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

    // フレーム落ちなどで移動量が大きくなっても通り越さないよう、
    // 攻撃範囲までの残り距離を上限にする。
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

    // 攻撃範囲判定と同じ距離基準を使用する。
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
