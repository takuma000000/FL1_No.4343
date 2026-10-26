#define NOMINMAX
#include "GameScene.h"
#include "AudioCatalog.h"
#include "TextureCatalog.h"
#include "ModelCatalog.h"
#include "CameraManager.h"
#include "ParticleManager.h"

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
	objectSettings_.Initialize(sphere_.get(), player_.get());
	InitializeSprite();
	InitializeCamera();

	/// ──────────────── パーティクル初期化 ───────────────
	auto particleManager = TKM::ParticleManager::GetInstance();
	particleManager->Initialize(dxCommon_, srvManager_, TKM::CameraManager::GetInstance()->GetMainCamera());

	/// ──────────────── ライトの初期化 ───────────────
	directionalLight_ = std::make_unique<DirectionalLight>();
	directionalLight_->Initialize({ 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, 1.0f);
}

void GameScene::Finalize() {

	/// ──────────────── オブジェクト設定終了 ───────────────
	objectSettings_.Finalize();

	/// ──────────────── 各種終了処理 ───────────────
	TextureManager::GetInstance()->Finalize();
	AudioManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();

	/// ──────────────── パーティクルの終了 ───────────────
	ParticleManager::GetInstance()->ClearAllGroups();
}

void GameScene::Update() {

	/// ──────────────── カメラ更新 ───────────────
	UpdateActiveCamera();

	/// ──────────────── 3Dオブジェクト更新 ───────────────
	// テスト用Sphere更新
	if (sphere_) {
		sphere_->Update();
	}
	if (player_) {
		player_->Update();
	}

	/// ──────────────── ライト更新 ───────────────
	if (directionalLight_) {
		directionalLight_->Update();
	}

	/// ──────────────── パーティクル更新 ───────────────
	constexpr float deltaTime = 1.0f / 60.0f;
	TKM::ParticleManager::GetInstance()->Update(deltaTime);

#ifdef USE_IMGUI

	ImGuiDebug();

#endif // 

}

void GameScene::Draw() {
	Draw3D();
	DrawSprite();
}

void GameScene::Draw3D() {
	/// ──────────────── 3Dオブジェクト描画 ───────────────
	Object3dCommon::GetInstance()->DrawSetCommon();

	// テスト用Sphere描画
	if (sphere_) {
		sphere_->Draw(dxCommon_);
	}

	if (player_) {
		player_->Draw(dxCommon_);
	}

	/// ──────────────── パーティクル描画 ───────────────
	ParticleManager::GetInstance()->Draw();
}

void GameScene::DrawSprite() {
	/// ──────────────── 2D描画共通設定 ───────────────
	TKM::SpriteCommon::GetInstance()->DrawSetCommon();
}

TKM::Camera* GameScene::UpdateActiveCamera() {
	auto activeCamera = TKM::CameraManager::GetInstance()->Update();

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
	sphere_->SetModel("plane.obj");
	// 座標・スケール設定
	constexpr Vector3 kSpherePosition = { 0.0f, 5.0f, 0.0f };
	constexpr Vector3 kSphereScale = { 3.0f, 3.0f, 3.0f };
	sphere_->SetTranslate(kSpherePosition);
	sphere_->SetScale(kSphereScale);

	// プレイヤー生成・初期化
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

	// メインカメラの設定を読み込み、更新する
	if (TKM::CameraManager::GetInstance()->GetMainCamera()) {
		TKM::CameraManager::GetInstance()->GetMainCamera()->SetSettingsPath("resources/data/mainCamera.json");
		TKM::CameraManager::GetInstance()->GetMainCamera()->LoadSettings();
		TKM::CameraManager::GetInstance()->GetMainCamera()->Update();
	}

	/// ───────────────── Objectカメラ設定 ───────────────
	if (sphere_) {
		sphere_->SetCamera(TKM::CameraManager::GetInstance()->GetMainCamera());
	}

	if (player_) {
		player_->SetCamera(TKM::CameraManager::GetInstance()->GetMainCamera());
	}
}

void GameScene::ImGuiDebug() {
#ifdef USE_IMGUI

	// ──────────────── カメラ表示 ───────────────
	// デバッグカメラの切り替え表示
	TKM::CameraManager::GetInstance()->ImGuiDebug();
	// メインカメラのデバッグ表示
	if (auto camera =
		TKM::CameraManager::GetInstance()->GetMainCamera()) {
		camera->ImGuiDebug(); // メインカメラのデバッグ表示
	}

	// ──────────────── オブジェクト編集 ───────────────
	objectSettings_.ImGuiDebug();

#endif
}