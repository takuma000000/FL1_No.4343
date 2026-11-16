#include "PlayerBullet.h"

void PlayerBullet::Initialize(TKM::DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity) {
	pos_ = position;
	velocity_ = velocity;
	isAlive_ = true;
	lifeTimer_ = 0;

	object_ = std::make_unique<TKM::Object3d>();
	object_->Initialize(TKM::Object3dCommon::GetInstance(), dxCommon);
	// ※プロジェクト内にある弾用モデル（またはsphere.objなど）に書き換えてください
	object_->SetModel("playerBullet/playerBullet.obj");
	object_->SetScale({ 0.7f, 0.7f, 0.7f });
	object_->SetTranslate(pos_);
	object_->SetRotate(rotation_);
	object_->Update();
}

void PlayerBullet::Update() {
	if (!isAlive_) {
		return;
	}

	// 速度分だけ移動
	pos_.x += velocity_.x;
	pos_.y += velocity_.y;
	pos_.z += velocity_.z;

	// 縦方向に回転させる
	rotation_.x += rotateSpeed_.x;

	// 寿命カウント（一定時間経過で消滅）
	lifeTimer_++;
	if (lifeTimer_ >= kMaxLifeTime) {
		isAlive_ = false;
	}

	if (object_) {
		object_->SetTranslate(pos_);
		object_->SetRotate(rotation_);
		object_->Update();
	}
}

void PlayerBullet::Draw(TKM::DirectXCommon* dxCommon) {
	if (!isAlive_) {
		return;
	}

	if (object_) {
		object_->Draw(dxCommon);
	}
}