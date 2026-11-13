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

    //============================================================
    // Getter
    //============================================================

    /// <summary>
    /// 敵の種類を取得する。
    /// </summary>
    /// <returns>敵の種類</returns>
    EnemyType GetType() const { return type_; }

    /// <summary>
    /// 敵の設定データを取得する。
    /// </summary>
    /// <returns>JSONから読み込まれた敵の設定データ</returns>
    const EnemyData& GetData() const { return data_; }

    /// <summary>
    /// 敵の現在座標を取得する。
    /// </summary>
    /// <returns>敵の現在座標</returns>
    const Vector3& GetPosition() const { return position_; }

    /// <summary>
    /// 敵の現在HPを取得する。
    /// </summary>
    /// <returns>敵の現在HP</returns>
    int GetCurrentHp() const { return currentHp_; }

    /// <summary>
    /// 敵の当たり判定半径を取得する。
    /// </summary>
    /// <returns>敵の当たり判定半径</returns>
    float GetCollisionRadius() const { return data_.collisionRadius; }

    /// <summary>
    /// 敵が生存しているか判定する。
    /// </summary>
    /// <returns>HPが1以上の場合はtrue</returns>
    bool IsAlive() const { return currentHp_ > 0; }

    /// <summary>
    /// ターゲットが攻撃範囲内にいるか判定する。
    /// </summary>
    /// <param name="targetPosition">ターゲットの座標</param>
    /// <returns>攻撃範囲内の場合はtrue</returns>
    bool IsWithinAttackRange(const Vector3& targetPosition) const;

    /// <summary>
    /// このフレームに攻撃が発生したか取得する。
    /// </summary>
    /// <returns>攻撃が発生した場合はtrue</returns>
    bool DidAttackThisFrame() const { return didAttackThisFrame_; }

    //============================================================
    // Setter
    //============================================================

    /// <summary>
    /// 敵の座標を設定する。
    /// </summary>
    /// <param name="position">設定する座標</param>
    void SetPosition(const Vector3& position) { position_ = position; }

private:
    /// <summary>
    /// 敵の種類に応じた移動を行う。
    /// </summary>
    void UpdateMovement(float deltaTime, const Vector3& targetPosition);

    /// <summary>
    /// ターゲットが索敵範囲内にいるか判定する。
    /// </summary>
    /// <param name="targetPosition">ターゲットの座標</param>
    /// <returns>索敵範囲内の場合はtrue</returns>
    bool IsWithinDetectionRange(const Vector3& targetPosition) const;

    /// <summary>
    /// 次の攻撃までの待機時間を更新する
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    void UpdateAttackCooldown(float deltaTime);

    /// <summary>
    /// 攻撃可能な状態であれば、攻撃を開始する。
    /// </summary>
    /// <param name="targetPosition">ターゲットの座標</param>
    void TryAttack(const Vector3& targetPosition);

private:
    // 描画対象。Enemyが所有権を持つ。
    std::unique_ptr<TKM::Object3d> object3d_;

    // 敵の種類と調整用パラメータ。
    EnemyType type_ = EnemyType::Normal;
    EnemyData data_{};

    // ゲーム中に変化する値。
    Vector3 position_{ 0.0f, 0.0f, 0.0f };
    int currentHp_ = 0;

    // 次の攻撃が可能になるまでの残り時間。
    // 0以下の場合は攻撃可能。
    float attackCooldownTimer_ = 0.0f;

    // 攻撃を行ったかどうかのフラグ。
    // 攻撃を行った場合は、次のフレームでfalseに戻す。
    bool didAttackThisFrame_ = false;
};
