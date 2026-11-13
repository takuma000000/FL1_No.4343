#include "SceneManager.h"

namespace TKM {
	void SceneManager::Initialize(DirectXCommon* dxCommon) {
		dxCommon_ = dxCommon;
		sceneTransition_.Initialize(dxCommon_);
	}

	void SceneManager::Update() {
		if (quitRequested_) { return; } // アプリ終了要求がある場合は更新しない

		// シーン切り替え用フェードを更新する
		sceneTransition_.Update();

		// 初回シーン、または完全に暗転したタイミングでシーンを切り替える
		if (nextScene_) {
			if (!scene_) {
				SwitchScene();
			} else if (sceneTransition_.IsCovered()) {
				SwitchScene();
				sceneTransition_.StartFadeIn();
			}
		}

		if (scene_) { // シーンが存在する場合のみ更新
			scene_->Update(); // シーンの更新
			scene_->FrameEnd(); // シーンのフレーム終了処理
		}
	}

	void SceneManager::Draw() {
		if (quitRequested_) { return; } // アプリ終了要求がある場合は描画しない
		if (scene_) { scene_->Draw(); } // シーンが存在する場合のみ描画
	}

	void SceneManager::Draw3D() {
		if (quitRequested_) { return; } // アプリ終了要求がある場合は描画しない
		if (!scene_) { return; } // シーンが存在しない場合は描画しない

		scene_->DrawBack(); // 背景スプライト（3Dより先）
		scene_->Draw3D();         // 3D描画
	}

	void SceneManager::DrawSprite() {
		if (quitRequested_) { return; } // アプリ終了要求がある場合は描画しない
		if (scene_) { scene_->DrawSprite(); } // シーンが存在する場合のみスプライト描画
	}

	void SceneManager::DrawTransition() {
		if (quitRequested_) { return; }
		sceneTransition_.Draw();
	}

	SceneManager::~SceneManager() {
		if (scene_) { // シーンが存在する場合のみ終了処理
			scene_->Finalize(); // 現在のシーンの終了処理
		}
	}

	void SceneManager::ChangeScene(const std::string& sceneName) {
		if (!sceneFactory_
			|| nextScene_
			|| sceneTransition_.IsActive()) {
			return;
		}

		auto nextScene = sceneFactory_->CreateScene(sceneName);
		if (!nextScene) {
			return;
		}

		nextScene_ = std::move(nextScene);

		if (scene_) {
			sceneTransition_.StartFadeOut();
		}
	}

	void SceneManager::SwitchScene() {
		if (!nextScene_) {
			return;
		}

		if (scene_) {
			scene_->Finalize();
		}

		scene_ = std::move(nextScene_);
		scene_->SetSceneManager(this);
		scene_->Start();
	}
}
