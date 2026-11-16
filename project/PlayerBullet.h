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

	// 生存フラグ（falseになったらPlayer側でリストから削除）
	bool IsAlive() const { return isAlive_; }
	void SetAlive(bool isAlive) { isAlive_ = isAlive; }

	const Vector3& GetPos() const { return pos_; }
	const TKM::Object3d* GetObject3d() const { return object_.get(); }

private:
	std::unique_ptr<TKM::Object3d> object_ = nullptr;
	Vector3 pos_ = { 0.0f, 0.0f, 0.0f };
	Vector3 velocity_ = { 0.0f, 0.0f, 1.5f };

	bool isAlive_ = true;
	int lifeTimer_ = 0;
	static constexpr int kMaxLifeTime = 120; // 120フレーム（約2秒）で自動消滅
};