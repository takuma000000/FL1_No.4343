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
	void ImGuiDebug();
#endif

	void SetCamera(TKM::Camera* camera);

	// 追従させるメインカメラとオフセット距離の設定
	void SetFollowCamera(TKM::Camera* camera, const Vector3& offset) {
		followCamera_ = camera;
		cameraOffset_ = offset;
	}

	// ──────────────── ゲッター / セッター ────────────────
	const Vector3& GetPos() const { return pos_; }
	void SetPos(const Vector3& pos) { pos_ = pos; }

	const Vector3& GetRotation() const { return rotation_; }
	void SetRotation(const Vector3& rotation) { rotation_ = rotation; }

	const Vector3& GetScale() const { return size_; }
	void SetScale(const Vector3& scale) { size_ = scale; }

	const TKM::Object3d* GetObject3d() const { return object_.get(); }

	bool IsVisible() const { return isVisible_; }
	void SetVisible(bool isVisible) { isVisible_ = isVisible; }

	float GetMoveSpeed() const { return moveSpeed_; }
	void SetMoveSpeed(float speed) { moveSpeed_ = speed; }

	float GetForwardSpeed() const { return forwardSpeed_; }
	void SetForwardSpeed(float speed) { forwardSpeed_ = speed; }

private:
	Vector3 pos_;
	Vector3 rotation_;
	Vector3 size_;

	float moveSpeed_ = 0.3f;     // 上下左右の移動速度
	float forwardSpeed_ = 0.5f;  // 奥（+Z方向）へ進む速度
	bool isVisible_ = true;

	std::unique_ptr<TKM::Object3d> object_ = nullptr;

	// 追従対象カメラと相対オフセット
	TKM::Camera* followCamera_ = nullptr;
	Vector3 cameraOffset_ = { 0.0f, 0.0f, -60.0f };
};