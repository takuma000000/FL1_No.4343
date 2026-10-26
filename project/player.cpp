#include "Player.h"
#include "Input.h"

Player::Player()
    : pos_(0.0f, 0.0f, 0.0f)
    , size_(1.0f, 1.0f, 1.0f)
    , forwardSpeed_(0.5f) // 前進スピードの設定
    , moveSpeed_(0.3f)    // 上下左右移動スピードの設定
{
}

Player::~Player() {
}

void Player::Initialize(TKM::DirectXCommon* dxCommon) {
    // 位置・スケールの初期化
    pos_ = { 0.0f, 0.0f, 0.0f };
    size_ = { 1.0f, 1.0f, 1.0f };

    // ──────────────── 3Dオブジェクトの作成と初期化 ────────────────
    object_ = std::make_unique<TKM::Object3d>();
    object_->Initialize(
        TKM::Object3dCommon::GetInstance(),
        dxCommon
    );

    // 使用するモデルの設定（モデル名やパスは環境に合わせて変更してください）
    object_->SetModel("sphere.obj");

    // トランスフォームの適用
    object_->SetTranslate(pos_);
    object_->SetScale(size_);
}

void Player::SetCamera(TKM::Camera* camera) {
    // 保持している 3D オブジェクトにカメラを設定
    if (object_) {
        object_->SetCamera(camera);
    }
}

void Player::Update() {
    
    if (object_) {
        object_->SetTranslate(pos_);
        object_->SetScale(size_); // ★ ここでも SetScale を呼び出す
        object_->Update();
    }

}

void Player::Draw(TKM::DirectXCommon* dxCommon) {
    // 3Dオブジェクトの描画関数を呼び出し
    if (object_) {
        object_->Draw(dxCommon);
    }
}