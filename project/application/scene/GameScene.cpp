#define NOMINMAX
#include "GameScene.h"
#include "AudioCatalog.h"
#include "TextureCatalog.h"
#include "ModelCatalog.h"
#include "CameraManager.h"
#include "ParticleManager.h"

#ifdef USE_IMGUI
#include "imgui.h"
#endif

using namespace TKM;

void GameScene::Initialize() {
	/// ──────────────── NULLチェック ────────────────
	assert(this != nullptr && "this is nullptr in GameScene::Initialize");
	assert(dxCommon_ != nullptr && "dxCommon is nullptr in GameScene::Initialize");

	/// ──────────────── 各種初期化処理 ───────────────
	AudioCatalog::LoadGameAudios();
	TextureCatalog::LoadTextureCatalogs();
	ModelCatalog::LoadModelCatalogs(dxCommon_);
	InitializeObjects();
	InitializeSprite();
	InitializeCamera();

	/// ──────────────── パーティクル初期化 ───────────────
	auto* particleManager = TKM::ParticleManager::GetInstance();
	particleManager->Initialize(dxCommon_, srvManager_, TKM::CameraManager::GetInstance()->GetMainCamera());

	/// ──────────────── ライトの初期化 ───────────────
	directionalLight_ = std::make_unique<DirectionalLight>();
	directionalLight_->Initialize({ 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, 1.0f);
}

void GameScene::Finalize() {
	/// ──────────────── 各種終了処理 ───────────────
	TextureManager::GetInstance()->Finalize();
	AudioManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();

	/// ──────────────── パーティクルの終了 ───────────────
	ParticleManager::GetInstance()->ClearAllGroups();
}

void GameScene::Update() {

	// 1. Player の更新
	if (player_) {
		player_->Update();
	}

	// 2. ImGui による Player Transform 操作 UI
	if (player_) {
		ImGui::Begin("Player Transform Inspector");

		// ──────────────── 位置（Position） ────────────────
		Vector3 pos = player_->GetPos();
		float p[3] = { pos.x, pos.y, pos.z };
		if (ImGui::DragFloat3("Position", p, 0.1f)) {
			player_->SetPos({ p[0], p[1], p[2] });
		}

		// ──────────────── 回転（Rotation: 0 ~ 360度） ────────────────
		Vector3 rot = player_->GetRotation();
		float r[3] = { rot.x, rot.y, rot.z };
		if (ImGui::DragFloat3("Rotation", r, 1.0f, -360.0f, 360.0f)) {
			player_->SetRotation({ r[0], r[1], r[2] });
		}

		// ──────────────── 拡大縮小（Scale: 0.001 ~ 10.0） ────────────────
		Vector3 scale = player_->GetScale();
		float s[3] = { scale.x, scale.y, scale.z };
		if (ImGui::DragFloat3("Scale", s, 0.01f, 0.001f, 10.0f, "%.3f")) {
			player_->SetScale({ s[0], s[1], s[2] });
		}

		ImGui::Separator();

		// ──────────────── 一括リセットボタン ────────────────
		if (ImGui::Button("Reset Transform")) {
			player_->SetPos({ 0.0f, 0.0f, 0.0f });
			player_->SetRotation({ 0.0f, 0.0f, 0.0f });
			player_->SetScale({ 0.5f, 0.5f, 0.5f }); // 現在表示できている適正スケール
		}

		ImGui::End();
	}

	/// ──────────────── カメラ更新 ───────────────
	UpdateActiveCamera();

	/// ──────────────── 3Dオブジェクト更新 ───────────────
	// テスト用Sphere更新
	sphere_->Update();
	player_->Update();

	/// ──────────────── ライト更新 ───────────────
	directionalLight_->Update();

	/// ──────────────── パーティクル更新 ───────────────
	constexpr float deltaTime = 1.0f / 60.0f;
	TKM::ParticleManager::GetInstance()->Update(deltaTime);

#ifdef USE_IMGUI
	ImGuiDebug();
#endif
}

void GameScene::Draw() {
	Draw3D();
	DrawSprite();
}

void GameScene::Draw3D() {

	/// ──────────────── 3Dオブジェクト描画 ───────────────
	Object3dCommon::GetInstance()->DrawSetCommon();
	// テスト用Sphere描画
	//sphere_->Draw(dxCommon_);

	if (player_) {
		player_->Draw(dxCommon_); // dxCommon_ を渡す
	}

	/// ──────────────── パーティクル描画 ───────────────
	ParticleManager::GetInstance()->Draw();
}

void GameScene::DrawSprite() {
	/// ──────────────── 2D描画共通設定 ───────────────
	TKM::SpriteCommon::GetInstance()->DrawSetCommon();
}

TKM::Camera* GameScene::UpdateActiveCamera() {
	auto* activeCamera =
		TKM::CameraManager::GetInstance()->Update();

	if (activeCamera) {
		TKM::ParticleManager::GetInstance()->SetCamera(activeCamera);
	}

	return activeCamera;
}

// *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// スプライトを作成し、初期化する
// *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
void GameScene::InitializeSprite() {}

// *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// 3Dオブジェクトを作成し、初期化する
// *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
void GameScene::InitializeObjects() {

	// テスト用Sphere生成
	sphere_ = std::make_unique<TKM::Object3d>();
	sphere_->Initialize(
		TKM::Object3dCommon::GetInstance(),
		dxCommon_
	);
	// モデル設定
	sphere_->SetModel("sphere.obj");
	// 座標・スケール設定
	constexpr Vector3 kSpherePosition = { 0.0f, 0.0f, 0.0f };
	constexpr Vector3 kSphereScale = { 3.0f, 3.0f, 3.0f };
	sphere_->SetTranslate(kSpherePosition);
	sphere_->SetScale(kSphereScale);

	player_ = std::make_unique<Player>();
	player_->Initialize(dxCommon_);

}

// *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// カメラを作成し、各オブジェクトに適用する
// *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
void GameScene::InitializeCamera() {
	/// ──────────────── カメラ初期値 ───────────────
	Vector3 mainRot = { 0.0f, 0.0f, 0.0f };
	Vector3 mainPos = { 0.0f, 0.0f, -60.0f };
	Vector3 debugTarget = { 0.0f, 0.0f, 0.0f };

	/// ──────────────── カメラマネージャー初期化 ───────────────
	TKM::CameraManager::GetInstance()->Initialize(mainRot, mainPos, debugTarget);
	// 行列を即更新して、1フレームだけ古いカメラになるのを防ぐ
	if (TKM::CameraManager::GetInstance()->GetMainCamera()) {
		TKM::CameraManager::GetInstance()->GetMainCamera()->Update();
	}

	/// ───────────────── Objectカメラ設定 ───────────────
	// Sphereにカメラを設定
	//sphere_->SetCamera(TKM::CameraManager::GetInstance()->GetMainCamera());

	if (player_) {
		player_->SetCamera(TKM::CameraManager::GetInstance()->GetMainCamera());
	}

}

void GameScene::ImGuiDebug() {
#ifdef USE_IMGUI

	// ──────────────── カメラ表示 ───────────────
	if (auto* camera = TKM::CameraManager::GetInstance()->GetMainCamera()) {
		camera->ImGuiDebug(); // メインカメラのデバッグ表示
	}

#endif
}

