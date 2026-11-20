#pragma once
#include "MyGame.h"
#include "Object3d.h"
#include "FollowCamera.h"
#include "../../PlayerBullet.h"
#include <memory>
#include <list>

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
		followCamera_.Initialize(camera, offset);
		followCamera_.Update(pos_);
	}

	// 追従パラメータ専用の編集画面（通常カメラの最終座標とは分離）
	void ImGuiFollowCamera() { followCamera_.ImGuiDebug(pos_); }

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

	// 発射された弾のリストを取得（当たり判定などで使用）
	const std::list<std::unique_ptr<PlayerBullet>>& GetBullets() const { return bullets_; }

	// 弾の発射処理
	void Attack();

	// ──────────────── HP関連のゲッター / セッター / 操作 ────────────────
	int GetHp() const { return hp_; }
	int GetMaxHp() const { return maxHp_; }
	void SetMaxHp(int maxHp) {
		maxHp_ = maxHp;
		if (hp_ > maxHp_) {
			hp_ = maxHp_;
		}
	}

	// ダメージを受ける処理（無敵時間中は無効）
	void Damage(int amount = 1);

private:

	// 弾生成・描画カメラ受け渡し用に保持するポインタ
	TKM::DirectXCommon* dxCommon_ = nullptr;
	TKM::Camera* camera_ = nullptr;

	Vector3 pos_;
	Vector3 rotation_;
	Vector3 size_;

	float moveSpeed_ = 0.3f;     // 上下左右の移動速度
	float forwardSpeed_ = 0.5f;  // 奥（+Z方向）へ進む速度
	bool isVisible_ = true;

	std::unique_ptr<TKM::Object3d> object_ = nullptr;

	// 追従対象カメラと相対オフセット
	TKM::FollowCamera followCamera_;
	Vector3 cameraOffset_ = { 0.0f, 0.0f, -60.0f };

	// 発射中の弾リスト
	std::list<std::unique_ptr<PlayerBullet>> bullets_;
	float bulletSpeed_ = 1.5f;   // 弾のスピード
	int shotCooldown_ = 0;       // 連射クールタイム用カウンタ

	// HP・無敵時間関連
	int maxHp_ = 5;              // 最大HP
	int hp_ = 5;                 // 現在のHP
	int invincibleTimer_ = 0;    // ダメージ後の無敵フレーム残り時間
	static constexpr int kMaxInvincibleTime = 60; // 被弾後の無敵時間（60フレーム = 約1秒）

};