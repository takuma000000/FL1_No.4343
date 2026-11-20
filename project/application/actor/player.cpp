#include "Player.h"
#include "Input.h"

Player::Player()
	: pos_(0.0f, 0.0f, 0.0f)
	, rotation_(0.0f, 0.0f, 0.0f)
	, size_(0.01f, 0.01f, 0.01f) {
}

Player::~Player() {
}

void Player::Initialize(TKM::DirectXCommon* dxCommon) {

	dxCommon_ = dxCommon;

	pos_ = { 0.0f, 0.0f, 0.0f };
	rotation_ = { 0.0f, 0.0f, 0.0f };
	size_ = { 1.0f, 1.0f, 1.0f };
	isVisible_ = true;

	// HP・無敵時間の初期化
	maxHp_ = 5;
	hp_ = maxHp_;
	invincibleTimer_ = 0;

	object_ = std::make_unique<TKM::Object3d>();
	object_->Initialize(
		TKM::Object3dCommon::GetInstance(),
		dxCommon
	);

	object_->SetModel("player/player.obj");

	// トランスフォーム適用
	object_->SetTranslate(pos_);
	object_->SetRotate(rotation_);
	object_->SetScale(size_);
	object_->Update();

	auto warmupBullet = std::make_unique<PlayerBullet>();
	warmupBullet->Initialize(dxCommon_, { 0.0f, -1000.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });
	warmupBullet->Update();
}

void Player::SetCamera(TKM::Camera* camera) {

	camera_ = camera;

	if (object_) {
		object_->SetCamera(camera);
	}

	// すでに飛んでいる弾にもカメラを適用（デバッグカメラ切り替え対応）
	for (auto& bullet : bullets_) {
		bullet->SetCamera(camera);
	}
}

void Player::Attack() {

	if (!dxCommon_ || hp_ <= 0) {
		return;
	}

	// プレイヤーの少し前方（+Z方向）から発射
	Vector3 spawnPos = { pos_.x, pos_.y, pos_.z + 2.0f };
	Vector3 velocity = { 0.0f, 0.0f, bulletSpeed_ };

	auto newBullet = std::make_unique<PlayerBullet>();
	newBullet->Initialize(dxCommon_, spawnPos, velocity);
	if (camera_) {
		newBullet->SetCamera(camera_);
	}

	bullets_.push_back(std::move(newBullet));
}

void Player::Damage(int amount) {

	// すでにHPが0以下か、無敵時間中ならダメージを受けない
	if (hp_ <= 0 || invincibleTimer_ > 0 || amount <= 0) {
		return;
	}

	hp_ -= amount;
	if (hp_ <= 0) {
		hp_ = 0;
		invincibleTimer_ = 0;
		isVisible_ = false;
		return;
	}

	// ダメージを受けたら無敵時間を開始
	invincibleTimer_ = kMaxInvincibleTime;
}

void Player::Update() {

	if (hp_ > 0) {
		// 無敵タイマーの更新と被弾中の点滅演出
		if (invincibleTimer_ > 0) {
			invincibleTimer_--;
			isVisible_ = (invincibleTimer_ % 8) < 4;
			if (invincibleTimer_ == 0) {
				isVisible_ = true;
			}
		}

		TKM::Input* input = TKM::Input::GetInstance();
		if (input != nullptr) {
			// A / D キーで X軸移動
			if (input->PushKey(DIK_A)) { pos_.x -= moveSpeed_; }
			if (input->PushKey(DIK_D)) { pos_.x += moveSpeed_; }

			// W / S キーで Y軸移動
			if (input->PushKey(DIK_W)) { pos_.y += moveSpeed_; }
			if (input->PushKey(DIK_S)) { pos_.y -= moveSpeed_; }

			// 上下矢印キーで Z軸（前後）移動
			if (input->PushKey(DIK_UP)) { pos_.z += moveSpeed_; }
			if (input->PushKey(DIK_DOWN)) { pos_.z -= moveSpeed_; }

			// スペースキーで弾発射
			if (shotCooldown_ > 0) {
				shotCooldown_--;
			}
			if (input->PushKey(DIK_SPACE) && shotCooldown_ <= 0) {
				Attack();
				shotCooldown_ = 10; // 10フレーム間隔で連射（単発にしたい場合はTriggerKeyに変更）
			}
		}
	} // ← 抜けていた if (hp_ > 0) の閉じ括弧

	// 追従カメラが設定されていれば、遅れなしで即座に一定距離を保って追従させる
	followCamera_.Update(pos_);

	// 位置・回転・スケールの最新値を 3D オブジェクトへセットして行列更新
	if (object_) {
		object_->SetTranslate(pos_);
		object_->SetRotate(rotation_);
		object_->SetScale(size_);
		object_->Update();
	}

	// 弾の更新と、消滅した弾の削除
	for (auto& bullet : bullets_) {
		bullet->Update();
	}
	bullets_.remove_if([](const std::unique_ptr<PlayerBullet>& bullet) {
		return !bullet->IsAlive();
		});
}

void Player::Draw(TKM::DirectXCommon* dxCommon) {

	// プレイヤー本体の描画（無敵点滅中やHP0時は非表示）
	if (isVisible_ && object_) {
		object_->Draw(dxCommon);
	}

	// 発射された弾の描画
	for (auto& bullet : bullets_) {
		bullet->Draw(dxCommon);
	}
}

#ifdef USE_IMGUI
void Player::ImGuiDebug() {
	// 1. 描画切り替え（Visible）
	ImGui::Checkbox("Visible", &isVisible_);

	ImGui::Separator();

	// 2. HP表示とテストボタン
	ImGui::Text("HP: %d / %d", hp_, maxHp_);
	if (invincibleTimer_ > 0) {
		ImGui::SameLine();
		ImGui::Text("(Invincible: %d)", invincibleTimer_);
	}

	if (ImGui::Button("Damage -1")) {
		Damage(1);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset HP")) {
		hp_ = maxHp_;
		invincibleTimer_ = 0;
		isVisible_ = true;
	}

	ImGui::Separator();

	// 3. 位置（Position）
	float p[3] = { pos_.x, pos_.y, pos_.z };
	if (ImGui::DragFloat3("Position", p, 0.1f)) {
		pos_ = { p[0], p[1], p[2] };
	}

	// 4. 回転（Rotation）
	float r[3] = { rotation_.x, rotation_.y, rotation_.z };
	if (ImGui::DragFloat3("Rotation", r, 0.01f)) {
		rotation_ = { r[0], r[1], r[2] };
	}

	// 5. 拡大縮小（Scale）
	float s[3] = { size_.x, size_.y, size_.z };
	if (ImGui::DragFloat3(
		"Scale",
		s,
		0.01f,
		0.001f,
		10.0f,
		"%.3f"
	)) {
		size_ = { s[0], s[1], s[2] };
	}

	ImGui::Separator();

	// 6. リセットボタン
	if (ImGui::Button("Reset Transform")) {
		pos_ = { 0.0f, 0.0f, 0.0f };
		rotation_ = { 0.0f, 0.0f, 0.0f };
		size_ = { 0.5f, 0.5f, 0.5f };
		hp_ = maxHp_;
		invincibleTimer_ = 0;
		isVisible_ = true;
	}
}
#endif