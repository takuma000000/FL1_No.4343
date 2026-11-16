#include "Enemy.h"

#include <algorithm>

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
    // 死亡後は移動やObject3dの更新を行わない。
    if (!IsAlive() || !object3d_) {
        return;
    }

    // 攻撃を行ったかどうかのフラグをリセットする。
    didAttackThisFrame_ = false;

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

void Enemy::TakeDamage(int damage) {
    // 0以下のダメージや、死亡後のダメージは無視する。
    if (damage <= 0 || !IsAlive()) {
        return;
    }

    currentHp_ = (std::max)(0, currentHp_ - damage);
}

bool Enemy::IsWithinAttackRange(const Vector3& targetPosition) const {
    const Vector3 difference = targetPosition - position_;
    const float attackRangeSquared = data_.attackRange * data_.attackRange;

    // 平方根を使わず、距離の二乗同士で判定する。
    return difference.lengthSquared() <= attackRangeSquared;
}

void Enemy::UpdateMovement(float deltaTime, const Vector3& targetPosition) {
    Vector3 difference = targetPosition - position_;

    // 地上敵は高さ方向へ追尾せず、XZ平面上だけを移動する。
    if (type_ != EnemyType::Flying) {
        difference.y = 0.0f;
    }

    // 攻撃可能な距離まで接近した場合は、
    // ターゲットを通り過ぎないように移動を停止する。
    if (IsWithinAttackRange(targetPosition)) {
        return;
    }

    const Vector3 direction = MyMath::SafeNormalize(
        difference,
        { 0.0f, 0.0f, 0.0f }
    );

    position_ += direction * data_.moveSpeed * deltaTime;
}

bool Enemy::IsWithinDetectionRange(const Vector3& targetPosition) const {
    // 敵からターゲットまでの座標差を求める
    const Vector3 difference = {
        targetPosition.x - position_.x,
        targetPosition.y - position_.y,
        targetPosition.z - position_.z
    };

    // 平方根を使わずに距離を比較するため、
    // ターゲットまでの距離の二乗を求める
    const float distanceSquared =
        difference.x * difference.x +
        difference.y * difference.y +
        difference.z * difference.z;

    // 索敵範囲についても二乗値を求める
    const float detectionRangeSquared =
        data_.detectionRange * data_.detectionRange;

    return distanceSquared <= detectionRangeSquared;
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