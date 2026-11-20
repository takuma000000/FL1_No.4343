#pragma once
#include "BaseScene.h"

#include "Object3d.h"
#include "Sprite.h"
#include "ObjectSettings.h"
#include "CollisionSettings.h"
#include "SpriteSettings.h"
#include "layout/SceneLayout.h"

#include <memory>

#include "player.h"
#include "../actor/enemy/data/EnemyDataCatalog.h"
#include "../actor/enemy/EnemyManager.h"

//=============================================================
// GameSceneクラス
// ゲーム本編を管理するシーンクラス。
//=============================================================
class GameScene : public TKM::BaseScene {
public:
	GameScene(TKM::DirectXCommon* dxCommon, TKM::SrvManager* srvManager)
		: dxCommon_(dxCommon),
		srvManager_(srvManager) {}

	~GameScene() = default;

	const char* GetSceneName() const override {
		return "GAME";
	}

	/// <summary>
	/// ゲーム本編シーンを初期化します。
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// ゲーム本編シーンを終了します。
	/// </summary>
	void Finalize() override;
	/// <summary>
	/// ゲーム本編シーンを更新します。
	/// </summary>
	void Update() override;
	/// <summary>
	/// ゲーム本編シーンを描画します。
	/// </summary>
	void Draw() override;
	/// <summary>
	/// ゲーム本編シーンの3Dオブジェクトを描画します。
	/// </summary>
	void Draw3D() override;
	/// <summary>
	/// ゲーム本編シーンのSpriteを描画します。
	/// </summary>
	void DrawSprite() override;
	/// <summary>
	/// ゲーム本編シーンの背景を描画します。
	/// </summary>
	void DrawBack() override;
	/// <summary>
	/// ImGuiでデバッグ表示を行います。
	/// </summary>
	void ImGuiDebug();

	/// <summary>
	/// スプライトを初期化します。
	/// </summary>
	void InitializeSprite();
	/// <summary>
	/// オブジェクトを初期化します。
	/// </summary>
	void InitializeObjects();
	/// <summary>
	/// カメラを初期化します。
	/// </summary>
	void InitializeCamera();

	/// <summary>
	/// アクティブなカメラを更新します。
	/// </summary>
	/// <returns></returns>
	TKM::Camera* UpdateActiveCamera();

	/// <summary>
	/// 敵データを読み込み、動作確認用の敵を生成します。
	/// </summary>
	void InitializeEnemies();

private:
	//======================================================================
	// 基本システム
	//======================================================================
	TKM::DirectXCommon* dxCommon_ = nullptr; // DirectX共通（デバイス/コマンド等）
	TKM::SrvManager* srvManager_ = nullptr; // SRV管理
	//======================================================================
	// カメラ / ライト / スカイボックス
	//======================================================================
	std::unique_ptr<TKM::DirectionalLight> directionalLight_ = nullptr; // ディレクショナルライト
	//======================================================================
	// テスト用3Dオブジェクト
	//======================================================================
	std::unique_ptr<TKM::Object3d> sphere_ = nullptr; // 球体

	std::unique_ptr<Player> player_ = nullptr;
	TKM::SceneLayout sceneLayout_;

	//======================================================================
	// 2Dスプライト
	//======================================================================
	std::unique_ptr<TKM::Sprite> logoSprite_ = nullptr; // TKMAXロゴ
	std::unique_ptr<TKM::Sprite> uvCheckerSprite_ = nullptr; // UVチェッカー

	//======================================================================
	// オブジェクト設定
	//======================================================================
	ObjectSettings objectSettings_; // オブジェクトの保存・編集設定
	CollisionSettings collisionSettings_; // オブジェクトの当たり判定設定
	SpriteSettings spriteSettings_; // スプライトの保存・編集設定

	//======================================================================
	// 敵データ・敵管理
	//======================================================================
	std::unique_ptr<EnemyDataCatalog> enemyDataCatalog_ = nullptr;
	std::unique_ptr<EnemyManager> enemyManager_ = nullptr;
};
