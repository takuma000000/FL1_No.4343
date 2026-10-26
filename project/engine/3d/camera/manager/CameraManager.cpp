#include "CameraManager.h"

#ifdef USE_IMGUI
#include "imgui.h"
#endif

namespace TKM {
	CameraManager* CameraManager::GetInstance() {
		static CameraManager instance;
		return &instance;
	}

	void CameraManager::Initialize(
		const Vector3& mainRotate,
		const Vector3& mainTranslate,
		const Vector3& debugTarget) {
		// メインカメラの初期化
		main_ = std::make_unique<Camera>();
		main_->SetRotate(mainRotate); // カメラの初期回転をセット
		main_->SetTranslate(mainTranslate); // カメラの初期位置をセット
		// デバッグカメラの初期化
		debug_ = std::make_unique<DebugCamera>();
		debug_->Initialize(main_->GetTranslate(), debugTarget); // デバッグカメラの初期位置はメインカメラと同じ、注視点は引数で指定
	
		active_ = main_.get(); // 最初は通常カメラをアクティブにしておく
		useDebug_ = false; // 最初はデバッグカメラを使用しない
	}

	Camera* CameraManager::Update() {
		if (!main_) { return nullptr; } // カメラが初期化されていない場合は nullptr を返す

		if (useDebug_ && debug_) { // デバッグカメラを使用する場合
			debug_->Update(); // デバッグカメラの更新
			active_ = debug_.get(); // アクティブカメラをデバッグカメラに切り替える
		} else { // 通常カメラを使用する場合
			main_->Update(); // 通常カメラの更新
			active_ = main_.get(); // アクティブカメラを通常カメラに切り替える
		}

		// 条件分岐してどちらかを返すのは微妙なので、キャッシュしておいた active_ を返す
		return active_;
	}

	void CameraManager::ImGuiDebug() {
#ifdef USE_IMGUI
		ImGui::Begin("デバッグカメラ");

		bool useDebugCamera = useDebug_;

		if (ImGui::Checkbox(
			"デバッグカメラを使用",
			&useDebugCamera
		)) {
			SetUseDebugCamera(useDebugCamera);
		}

		ImGui::Separator();

		if (useDebug_ && debug_) {
			// 操作中は緑、停止中は灰色で表示する
			auto ShowControlState = [](
				const char* text,
				bool isActive
				) {
					const ImVec4 activeColor = {
						0.2f,
						1.0f,
						0.2f,
						1.0f
					};

					const ImVec4 inactiveColor =
						ImGui::GetStyleColorVec4(
							ImGuiCol_TextDisabled
						);

					ImGui::TextColored(
						isActive
						? activeColor
						: inactiveColor,
						"%s",
						text
					);
				};

			ShowControlState(
				"右ドラッグ : 視点回転",
				debug_->IsRotating()
			);

			ShowControlState(
				"左ドラッグ : 平行移動",
				debug_->IsPanning()
			);

			ShowControlState(
				"ホイール   : 前後移動",
				debug_->IsZooming()
			);
		} else {
			ImGui::TextDisabled(
				"メインカメラを使用中"
			);
		}

		ImGui::End();
#endif
	}

	Camera* CameraManager::GetActiveCamera() const { return active_; } // Updateのたびに条件分岐してどちらかを返すのは微妙なので、キャッシュしておいた active_ を返す
	void CameraManager::SetUseDebugCamera(bool enable) { useDebug_ = enable; } // デバッグカメラの使用を切り替える
	bool CameraManager::IsUsingDebugCamera() const { return useDebug_; } // デバッグカメラを使用しているかどうかを取得する
	Camera* CameraManager::GetMainCamera() const { return main_.get(); } // 通常カメラを取得する
}