#define NOMINMAX
#include "GameScene.h"
#include "AudioCatalog.h"
#include "TextureCatalog.h"
#include "ModelCatalog.h"
#include "CameraManager.h"
#include "ParticleManager.h"

#include "Logger.h"

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

	InitializeEnemies(); // カメラを使用するため、カメラの初期化完了後に生成する

	/// ──────────────── パーティクル初期化 ───────────────
	auto* particleManager = TKM::ParticleManager::GetInstance();
	particleManager->Initialize(dxCommon_, srvManager_, TKM::CameraManager::GetInstance()->GetMainCamera());

	/// ──────────────── ライトの初期化 ───────────────
	directionalLight_ = std::make_unique<DirectionalLight>();
	directionalLight_->Initialize({ 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, 1.0f);
}

void GameScene::Finalize() {
	/// ──────────────── 敵の終了処理 ────────────────
	if (enemyManager_) {
		// EnemyとEnemyが所有するObject3dを破棄する。
		enemyManager_->Clear();
		enemyManager_.reset();
	}

    // EnemyManagerはCatalogを参照しているので、Managerを破棄した後にCatalogを破棄する。
	enemyDataCatalog_.reset();

	/// ──────────────── 各種終了処理 ───────────────
	TextureManager::GetInstance()->Finalize();
	AudioManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();

	/// ──────────────── パーティクルの終了 ───────────────
	ParticleManager::GetInstance()->ClearAllGroups();
}

void GameScene::Update() {
	// 現在は60FPS固定として経過時間を扱う。
	// 将来的には実際のフレーム経過時間へ置き換える。
	constexpr float kDeltaTime = 1.0f / 60.0f;

	/// ──────────────── カメラ更新 ───────────────
	UpdateActiveCamera();

	/// ──────────────── 3Dオブジェクト更新 ───────────────
	// テスト用Sphere更新
	sphere_->Update();

    /// ──────────────── 敵更新 ───────────────
	if (enemyManager_) {
		// 現在は中央のSphereをターゲットとしてEnemyを更新する。
		const Vector3 targetPosition = sphere_->GetTranslate();

		enemyManager_->Update(kDeltaTime, targetPosition);
	}

	/// ──────────────── ライト更新 ───────────────
	directionalLight_->Update();

	/// ──────────────── パーティクル更新 ───────────────
	TKM::ParticleManager::GetInstance()->Update(kDeltaTime);

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
	sphere_->Draw(dxCommon_);

	/// ──────────────── 敵描画 ───────────────
	if (enemyManager_) {
		// EnemyManagerが所有する生存中のEnemyを描画する。
		enemyManager_->Draw();
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
	sphere_->SetCamera(TKM::CameraManager::GetInstance()->GetMainCamera());
}

void GameScene::ImGuiDebug() {
#ifdef USE_IMGUI

	// ──────────────── カメラ表示 ───────────────
	if (auto* camera = TKM::CameraManager::GetInstance()->GetMainCamera()) {
		camera->ImGuiDebug(); // メインカメラのデバッグ表示
	}
#endif
}

// *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// 敵データを読み込み、動作確認用のEnemyを生成する
// *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
void GameScene::InitializeEnemies() {
	//=========================================================
	// EnemyDataCatalogの生成
	//=========================================================
	enemyDataCatalog_ =
		std::make_unique<EnemyDataCatalog>();

	std::string errorMessage;

	// resources/data/enemy内にある
	// .enemy.jsonファイルをすべて読み込む。
	const bool isCatalogLoaded =
		enemyDataCatalog_->LoadDirectory(
			"resources/data/enemy",
			errorMessage
		);

	if (!isCatalogLoaded) {
		Logger::Log(
			"[Enemy] Failed to load enemy data directory.\n" +
			errorMessage +
			"\n"
		);

		return;
	}

	//=========================================================
	// カメラの取得
	//=========================================================
	TKM::Camera* mainCamera =
		TKM::CameraManager::GetInstance()->GetMainCamera();

	// EnemyのObject3dへ設定するカメラがない場合は、
	// 描画できないためEnemyManagerを生成しない。
	if (!mainCamera) {
		Logger::Log(
			"[Enemy] Main camera is null.\n"
		);

		return;
	}

	//=========================================================
	// EnemyManagerの初期化
	//=========================================================
	enemyManager_ =
		std::make_unique<EnemyManager>();

	enemyManager_->Initialize(
		dxCommon_,
		mainCamera,
		*enemyDataCatalog_
	);

	//=========================================================
	// BasicVirusの生成
	//=========================================================
	// 中央のテスト用Sphereと重ならないよう、
	// X方向に離れた位置へ生成する。
	constexpr Vector3 kBasicVirusPosition = {
		-15.0f,
		0.0f,
		0.0f
	};

	const bool isSpawned =
		enemyManager_->Spawn(
			"BasicVirus",
			kBasicVirusPosition,
			errorMessage
		);

	if (!isSpawned) {
		Logger::Log(
			"[Enemy] Failed to spawn BasicVirus.\n" +
			errorMessage +
			"\n"
		);

		return;
	}

	Logger::Log(
		"[Enemy] BasicVirus spawned successfully.\n"
	);
}