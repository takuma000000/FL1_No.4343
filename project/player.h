#pragma once
#include "MyGame.h"
#include "Object3d.h"
#include <memory>

class Input;

class Player {
public:
    Player();
    ~Player();

    void Initialize(TKM::DirectXCommon* dxCommon);
    void Update();
    void Draw(TKM::DirectXCommon* dxCommon);

#ifdef USE_IMGUI
    // ImGui描画処理（GameScene等から呼び出す）
    void ImGuiDebug();
#endif

    void SetCamera(TKM::Camera* camera);

    // ──────────────── ゲッター / セッター ────────────────
    const Vector3& GetPos() const { return pos_; }
    void SetPos(const Vector3& pos) { pos_ = pos; }

    const Vector3& GetRotation() const { return rotation_; }
    void SetRotation(const Vector3& rotation) { rotation_ = rotation; }

    const Vector3& GetScale() const { return size_; }
    void SetScale(const Vector3& scale) { size_ = scale; }

    // 描画フラグのゲッター / セッター
    bool IsVisible() const { return isVisible_; }
    void SetVisible(bool isVisible) { isVisible_ = isVisible; }

    float GetMoveSpeed() const { return moveSpeed_; }
    void SetMoveSpeed(float speed) { moveSpeed_ = speed; }

private:
    Vector3 pos_;
    Vector3 rotation_;
    Vector3 size_;

    float moveSpeed_ = 0.3f;
    bool isVisible_ = true;

    std::unique_ptr<TKM::Object3d> object_ = nullptr;
};