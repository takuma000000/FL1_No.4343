#pragma once
#include "BaseScene.h"

#include "Object3d.h"
#include "ObjectSave.h"

#include <memory>

#include "../../player.h"

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
	/// オブジェクトの保存設定を初期化します。
	/// </summary>
	void InitializeObjectSave();

	/// <summary>
	/// アクティブなカメラを更新します。
	/// </summary>
	/// <returns></returns>
	TKM::Camera* UpdateActiveCamera();

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

	//======================================================================
	// オブジェクトの保存設定
	//======================================================================
	TKM::ObjectSave objectSave_; // オブジェクトの保存設定

private:
	//======================================================================
	// ImGuiオブジェクト選択
	//======================================================================
	enum class SelectedObject {
		None,   // 選択なし
		Player, // Player
		Sphere  // Sphere
	};
	// ImGuiで選択されているオブジェクト
	SelectedObject selectedObject_ = SelectedObject::None;
};