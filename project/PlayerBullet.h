#pragma once
#include "MyGame.h"
#include "Object3d.h"
#include <memory>

class PlayerBullet {
public:
	void Initialize(TKM::DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity);
	void Update();
	void Draw(TKM::DirectXCommon* dxCommon);

	void SetCamera(TKM::Camera* camera) {
		if (object_) {
			object_->SetCamera(camera);
		}
	}

	// 回転速度（ラジアン）を外部から変更したい場合用
	void SetRotateSpeed(const Vector3& rotateSpeed) { rotateSpeed_ = rotateSpeed; }

	// 生存フラグ（falseになったらPlayer側でリストから削除）
	bool IsAlive() const { return isAlive_; }
	void SetAlive(bool isAlive) { isAlive_ = isAlive; }

	const Vector3& GetPos() const { return pos_; }
	const TKM::Object3d* GetObject3d() const { return object_.get(); }

private:
	std::unique_ptr<TKM::Object3d> object_ = nullptr;
	Vector3 pos_ = { 0.0f, 0.0f, 0.0f };
	Vector3 velocity_ = { 0.0f, 0.0f, 1.5f };
	Vector3 rotation_ = { 0.0f, 0.0f, 0.0f };          // 現在の回転角（ラジアン）
	Vector3 rotateSpeed_ = { 0.1f, 0.0f, 0.0f };       // 縦回転（X軸）の回転速度

	bool isAlive_ = true;
	int lifeTimer_ = 0;
	static constexpr int kMaxLifeTime = 120; // 120フレーム（約2秒）で自動消滅
};