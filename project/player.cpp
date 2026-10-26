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

    if (!isVisible_) {
        return;
    }

    if (object_) {
        object_->Draw(dxCommon);
    }
}

#ifdef USE_IMGUI
void Player::ShowImGui() {
    ImGui::Begin("Player Control Panel");

    // 1. 描画切り替え（Visible）
    ImGui::Checkbox("Visible", &isVisible_);

    ImGui::Separator();

    // 2. 位置（Position）
    float p[3] = { pos_.x, pos_.y, pos_.z };
    if (ImGui::DragFloat3("Position", p, 0.1f)) {
        pos_ = { p[0], p[1], p[2] };
    }

    // 3. 回転（Rotation）
    float r[3] = { rotation_.x, rotation_.y, rotation_.z };
    if (ImGui::DragFloat3("Rotation", r, 1.0f, -360.0f, 360.0f)) {
        rotation_ = { r[0], r[1], r[2] };
    }

    // 4. 拡大縮小（Scale）
    float s[3] = { size_.x, size_.y, size_.z };
    if (ImGui::DragFloat3("Scale", s, 0.01f, 0.001f, 10.0f, "%.3f")) {
        size_ = { s[0], s[1], s[2] };
    }

    ImGui::Separator();

    // 5. リセットボタン
    if (ImGui::Button("Reset Transform")) {
        pos_ = { 0.0f, 0.0f, 0.0f };
        rotation_ = { 0.0f, 0.0f, 0.0f };
        size_ = { 0.5f, 0.5f, 0.5f };
        isVisible_ = true;
    }

    ImGui::End();
}
#endif