#include "ParticleEditorScene.h"
#include "CameraManager.h"
#include "LineRenderer.h"
#include "ParticleEffects.h"
#include "ParticleManager.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif

void ParticleEditorScene::Initialize() {
	auto* cameras = TKM::CameraManager::GetInstance();
	cameras->Initialize({ 0.0f, 0.0f, 0.0f }, { 0.0f, 3.0f, -12.0f }, { 0.0f, 0.0f, 0.0f });
	cameras->SetUseDebugCamera(true);
	auto* particles = TKM::ParticleManager::GetInstance();
	particles->ClearAllGroups();
	particles->Initialize(dxCommon_, srvManager_, cameras->Update());
	editor_.Initialize();
}
void ParticleEditorScene::Finalize() {
	TKM::ParticleEffects::GetInstance()->StopPreview();
	TKM::ParticleManager::GetInstance()->ClearAllGroups();
}
void ParticleEditorScene::Update() {
	editor_.ImGuiDebug();
#ifdef USE_IMGUI
	ImGui::Begin("プレビュー表示");
	ImGui::Checkbox("床グリッド", &showGrid_);
	ImGui::TextUnformatted("右ドラッグ: 回転 / 左ドラッグ: 平行移動 / ホイール: 前後移動");
	if (ImGui::Button("カメラを初期位置へ")) {
		auto* cameras = TKM::CameraManager::GetInstance();
		cameras->Initialize({ 0, 0, 0 }, { 0, 3, -12 }, { 0, 0, 0 });
		cameras->SetUseDebugCamera(true);
	}
	ImGui::End();
#endif
	auto* particles = TKM::ParticleManager::GetInstance();
	particles->SetCamera(TKM::CameraManager::GetInstance()->Update());
	particles->Update(editor_.IsPaused() ? 0.0f : -1.0f);
}
void ParticleEditorScene::Draw3D() {
#ifdef _DEBUG
	if (showGrid_) {
		auto* lines = TKM::LineRenderer::GetInstance();
		lines->BeginFrame();
		constexpr int kGridHalfSize = 10;
		const TKM::LineRenderer::Color grey = { 0.25f, 0.25f, 0.25f, 1.0f };
		for (int i = -kGridHalfSize; i <= kGridHalfSize; ++i) {
			const float p = static_cast<float>(i), size = static_cast<float>(kGridHalfSize);
			lines->AddLine({ p, 0, -size }, { p, 0, size }, i == 0 ? TKM::LineRenderer::Color{ 0, 0.4f, 1, 1 } : grey);
			lines->AddLine({ -size, 0, p }, { size, 0, p }, i == 0 ? TKM::LineRenderer::Color{ 1, 0.2f, 0.2f, 1 } : grey);
		}
		lines->AddLine({ 0, 0, 0 }, { 0, 3, 0 }, { 0, 1, 0, 1 });
		lines->Draw(TKM::CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix());
	}
#endif
	TKM::ParticleManager::GetInstance()->Draw();
}
