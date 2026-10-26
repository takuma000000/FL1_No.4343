#pragma once
#include "MyGame.h"
#include "Object3d.h"
#include "Camera.h"
#include <memory>

// 前方宣言（循環参照防止）
class Input;

class Player {
public:
    Player();
    ~Player();

    /// <summary>
    /// プレイヤーの初期化処理
    /// </summary>
    /// <param name="dxCommon">DirectX共通オブジェクト</param>
    void Initialize(TKM::DirectXCommon* dxCommon);

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw(TKM::DirectXCommon* dxCommon);

    // ──────────────── セッター / ゲッター ────────────────

    /// <summary>
    /// 入力オブジェクトを設定
    /// </summary>
    void SetInput(Input* input) { input_ = input; }

    /// <summary>
    /// 描画用カメラを設定
    /// </summary>
    void SetCamera(TKM::Camera* camera);

    /// <summary>
    /// プレイヤーの現在座標を取得（カメラ追従などで使用）
    /// </summary>
    const Vector3& GetPos() const { return pos_; }

private:
    // 座標・スケール
    Vector3 pos_;
    Vector3 size_;

    // 移動スピードの設定
    float forwardSpeed_; // 奥方向（Z軸）への自動前進スピード
    float moveSpeed_;    // 上下左右（X/Y軸）のキー移動スピード

    // 内部で管理する 3D オブジェクト
    std::unique_ptr<TKM::Object3d> object_ = nullptr;

    // 外部からの参照ポインタ
    Input* input_ = nullptr;
};