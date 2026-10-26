#pragma once
#include "MyGame.h"
#include "Object3d.h"
#include <memory>

class Player {
public:
    Player();
    ~Player();

    void Initialize(TKM::DirectXCommon* dxCommon);
    void Update();
    void Draw(TKM::DirectXCommon* dxCommon);

    void SetCamera(TKM::Camera* camera);

    // ──────────────── ゲッター / セッター ────────────────
    const Vector3& GetPos() const { return pos_; }
    void SetPos(const Vector3& pos) { pos_ = pos; }

    const Vector3& GetRotation() const { return rotation_; }
    void SetRotation(const Vector3& rotation) { rotation_ = rotation; }

    const Vector3& GetScale() const { return size_; }
    void SetScale(const Vector3& scale) { size_ = scale; }

private:
    Vector3 pos_;
    Vector3 rotation_;
    Vector3 size_;

    std::unique_ptr<TKM::Object3d> object_ = nullptr;
};