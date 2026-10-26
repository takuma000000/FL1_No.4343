#include "Player.h"

Player::Player()
    : pos_(0.0f, 0.0f, 0.0f)
    , rotation_(0.0f, 0.0f, 0.0f)
    , size_(0.01f, 0.01f, 0.01f)
{
}

Player::~Player() {
}

void Player::Initialize(TKM::DirectXCommon* dxCommon) {
    pos_ = { 0.0f, 0.0f, 0.0f };
    rotation_ = { 0.0f, 0.0f, 0.0f };
    size_ = { 1.0f, 1.0f, 1.0f };

    object_ = std::make_unique<TKM::Object3d>();
    object_->Initialize(
        TKM::Object3dCommon::GetInstance(),
        dxCommon
    );

    object_->SetModel("sphere.obj");

    // トランスフォーム適用
    object_->SetTranslate(pos_);
    object_->SetRotate(rotation_);
    object_->SetScale(size_);
    object_->Update();
}

void Player::SetCamera(TKM::Camera* camera) {
    if (object_) {
        object_->SetCamera(camera);
    }
}

void Player::Update() {

    // 位置・回転・スケールの最新値を 3D オブジェクトへセットして行列更新
    if (object_) {
        object_->SetTranslate(pos_);
        object_->SetRotate(rotation_);
        object_->SetScale(size_);
        object_->Update();
    }
}

void Player::Draw(TKM::DirectXCommon* dxCommon) {
    if (object_) {
        object_->Draw(dxCommon);
    }
}