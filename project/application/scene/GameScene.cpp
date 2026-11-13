#define NOMINMAX
#include "GameScene.h"
#include "AudioCatalog.h"
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
	ModelCatalog::LoadModelCatalogs(dxCommon_);
	InitializeObjects();
	collisionSettings_.Initialize(sphere_.get(), player_.get());
	objectSettings_.Initialize(sphere_.get(), player_.get());
	InitializeSprite();
	spriteSettings_.Initialize();
	InitializeCamera();

	/// ──────────────── パーティクル初期化 ───────────────
	auto particleManager = TKM::ParticleManager::GetInstance();
	particleManager->Initialize(dxCommon_, srvManager_, TKM::CameraManager::GetInstance()->GetMainCamera());

	/// ──────────────── ライトの初期化 ───────────────
	directionalLight_ = std::make_unique<DirectionalLight>();
	directionalLight_->Initialize({ 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, 1.0f);
}

void GameScene::Finalize() {
	/// ──────────────── スプライト設定終了 ───────────────
	spriteSettings_.Finalize();

	/// ──────────────── 当たり判定設定終了 ───────────────
	collisionSettings_.Finalize();

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

	/// ──────────────── 3Dオブジェクト更新 ───────────────
	// 先にプレイヤーを更新し、同時に追従カメラの座標を確定させる
	if (player_) {
		player_->Update();
	}

	/// ──────────────── カメラ更新 ───────────────
	// 確定したカメラ座標をもとに同じフレーム内で行列更新・適用（1フレームの遅れも防止）
	UpdateActiveCamera();

	// テスト用Sphere更新
	if (sphere_) {
		sphere_->Update();
	}

	/// ──────────────── スプライト更新 ───────────────
	logoSprite_->Update();      // TKMAXロゴ更新
	uvCheckerSprite_->Update(); // UVチェッカー更新

	/// ──────────────── ライト更新 ───────────────
	if (directionalLight_) {
		directionalLight_->Update();
	}

	/// ──────────────── パーティクル更新 ───────────────
	TKM::ParticleManager::GetInstance()->Update();

#ifdef USE_IMGUI
	ImGuiDebug();
#endif
}

void GameScene::Draw() {
	DrawBack();
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

	/// ──────────────── 前面スプライト描画 ───────────────
	uvCheckerSprite_->Draw(); // UVチェッカー描画
}

void GameScene::DrawBack() {
	/// ──────────────── 背景描画共通設定 ───────────────
	TKM::SpriteCommon::GetInstance()->DrawSetCommon();

	/// ──────────────── TKMAXロゴ背景描画 ───────────────
	logoSprite_->Draw();
}

TKM::Camera* GameScene::UpdateActiveCamera() {
	auto activeCamera = TKM::CameraManager::GetInstance()->Update();

	if (activeCamera) {
		TKM::ParticleManager::GetInstance()->SetCamera(activeCamera);

		sphere_->SetCamera(activeCamera); // テスト用Sphereにカメラを設定
		player_->SetCamera(activeCamera); // プレイヤーにカメラを設定
	}

	return activeCamera;
}

// *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// スプライトを作成し、初期化する
// *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
void GameScene::InitializeSprite() {
	/// TKMAXロゴを生成する
	logoSprite_ = std::make_unique<TKM::Sprite>();
	logoSprite_->Initialize(
		TKM::SpriteCommon::GetInstance(),
		dxCommon_,
		"./resources/texture/TKMAX_logo.png"
	);

	// 画像の中心を画面中央へ配置する
	logoSprite_->SetAnchorPoint({ 0.5f, 0.5f });
	logoSprite_->SetPosition({
		static_cast<float>(TKM::WindowsAPI::GetClientWidth()) * 0.5f,
		static_cast<float>(TKM::WindowsAPI::GetClientHeight()) * 0.5f
		});

	// 元画像と同じサイズで表示する
	logoSprite_->SetAutoAdjustTextureSize(false);
	logoSprite_->SetTextureSize({ 459.0f, 287.0f });
	logoSprite_->SetSize({ 459.0f, 287.0f });
	logoSprite_->Update();

	/// UVチェッカーを生成する
	uvCheckerSprite_ = std::make_unique<TKM::Sprite>();
	uvCheckerSprite_->Initialize(
		TKM::SpriteCommon::GetInstance(),
		dxCommon_,
		"./resources/texture/uvChecker.png"
	);

	// 画像の中心を画面中央へ配置する
	uvCheckerSprite_->SetAnchorPoint({ 0.5f, 0.5f });
	uvCheckerSprite_->SetPosition({
		static_cast<float>(TKM::WindowsAPI::GetClientWidth()) * 0.5f,
		static_cast<float>(TKM::WindowsAPI::GetClientHeight()) * 0.5f
		});

	// 元画像と同じサイズで表示する
	uvCheckerSprite_->SetAutoAdjustTextureSize(false);
	uvCheckerSprite_->SetTextureSize({ 512.0f, 512.0f });
	uvCheckerSprite_->SetSize({ 512.0f, 512.0f });
	uvCheckerSprite_->Update();
}

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
	auto mainCamera = TKM::CameraManager::GetInstance()->GetMainCamera();
	if (mainCamera) {
		mainCamera->SetSettingsPath("resources/data/mainCamera.json");
		mainCamera->LoadSettings();
		mainCamera->Update();
	}

	/// ───────────────── Objectカメラ設定 ───────────────
	if (sphere_) {
		sphere_->SetCamera(mainCamera);
	}

	if (player_) {
		player_->SetCamera(mainCamera);
		if (mainCamera) {
			player_->SetFollowCamera(mainCamera, mainCamera->GetTranslate());
		}
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

	// ──────────────── スプライト編集 ───────────────
	spriteSettings_.ImGuiDebug();
#endif
}
