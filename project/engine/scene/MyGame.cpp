#include "MyGame.h"

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include "ImGuiManager.h"
#include "CameraManager.h"
#include "CollisionManager.h"
#include "LineRenderer.h"
#include "DeltaTimeManager.h"

namespace TKM {
	void MyGame::Initialize() {

		//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
		TKM::Framework::Initialize(); //基底クラスの初期化処理
		postEffectEditor_.Initialize(dxCommon_.get());

		// Initialize sceneManager_
		sceneManager_ = std::make_unique<TKM::SceneManager>();
		sceneManager_->Initialize(dxCommon_.get());

		//シーンファクトリーの生成、マネージャにセット
		sceneFactory_ = std::make_unique<TKM::SceneFactory>(dxCommon_.get(), srvManager_.get());
		sceneManager_->SetSceneFactory(sceneFactory_.get());

		//最初のシーンを設定
		sceneManager_->SetNextScene(
			std::unique_ptr<TKM::BaseScene>(sceneFactory_->CreateScene("GAME"))
		);

		assert(dxCommon_.get() != nullptr && "DirectXCommon is nullptr in MyGame::Initialize");
		assert(srvManager_.get() != nullptr && "SrvManager is nullptr in MyGame::Initialize");

		// ImGuiManagerの初期化
		imguiManager_ = std::make_unique<TKM::ImGuiManager>();
		imguiManager_->Initialize(windowsAPI_.get(), dxCommon_.get());
		//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

	}

	void MyGame::Finalize() {
		////*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
		////				解放
		////*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

		// 終了処理
		postEffectEditor_.Finalize();
		imguiManager_->Finalize();

		//基底クラスの終了処理
		TKM::Framework::Finalize();

	}

	void MyGame::Update() {
		//---------------------------------------------------------

		//基底クラスの更新処理
		TKM::Framework::Update();

		// ** ImGui処理開始 **
		imguiManager_->Begin();

		//シーンマネージャーの更新
		sceneManager_->Update();

		// シーン内オブジェクトの更新後に当たり判定を行う
		// 最新の位置とサイズを使って判定するため、この位置で呼び出す
		CollisionManager::GetInstance()->Update();

		// ウィンドウメッセージの処理
		if (windowsAPI_->ProcessMessage()) {
			SetEndRequest(true); // Frameworkの終了フラグを設定
		}

		postEffectEditor_.Update(DeltaTimeManager::GetInstance()->GetDeltaTime());

		// ** ImGui処理終了 **
		imguiManager_->End();

		viewport_ = dxCommon_->GetViewport();
		scissorRect_ = dxCommon_->GetRect();

		//---------------------------------------------------------
	}

	void MyGame::Draw() {
		if (const auto* camera = CameraManager::GetInstance()->GetActiveCamera()) {
			dxCommon_->SetComicProjection(camera->GetProjectionMatrix());
		}

		// ① 3Dを RenderTexture に描く（ポスト対象）
		dxCommon_->PreDraw();
		srvManager_->PreDraw();
		sceneManager_->Draw3D();

#ifdef _DEBUG
		// 登録されているコライダーのラインを描画予約する
		if (auto* activeCamera =
			CameraManager::GetInstance()->GetActiveCamera()) {
			CollisionManager::GetInstance()->DrawDebug(
				activeCamera->GetViewProjectionMatrix()
			);
		}
#endif

		// ② Swapchain に切り替え
		dxCommon_->BeginDrawToSwapchain();

		// ③ RenderTexture → Swapchain（ポスト適用）
		srvManager_->PreDraw();
		dxCommon_->DrawPostEffectToSwapchain();

#ifdef _DEBUG
		// 予約したグリッド・当たり判定ラインは、ポスト処理後にまとめて描画する。
		LineRenderer::GetInstance()->Flush();
#endif

		// ④ UI(Sprite)を Swapchain に直描き（ポスト対象外）
		srvManager_->PreDraw();
		sceneManager_->DrawSprite();

		// ⑤ シーン切り替えフェードを最前面へ描画
		sceneManager_->DrawTransition();

		// ⑥ ImGui（デバッグ操作はフェード中も表示する）
		imguiManager_->Draw();

		// ⑦ フレーム終了
		dxCommon_->PostDraw();
	}
}
