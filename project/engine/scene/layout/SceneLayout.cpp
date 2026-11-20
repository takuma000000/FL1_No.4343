#include "SceneLayout.h"
#include "Object3dCommon.h"
#include <cstdio>
#include <exception>
#ifdef USE_IMGUI
#include "imgui.h"
#endif
namespace TKM {
	void SceneLayout::Initialize(DirectXCommon* dxCommon, Camera* camera) { dxCommon_ = dxCommon; camera_ = camera; }
	bool SceneLayout::Load(const std::string& filePath) {
		if (!dxCommon_) { message_ = "SceneLayout::Initializeを先に呼んでください"; return false; }
		SceneLayoutData data; std::string error;
		if (!LoadSceneLayoutData(filePath, data, error)) { message_ = error; return false; }
		try {
			std::vector<Object> objects;
			objects.reserve(data.objects.size());
			for (const auto& entry : data.objects) {
				auto object = std::make_unique<Object3d>();
				object->Initialize(Object3dCommon::GetInstance(), dxCommon_);
				object->SetModel(entry.model);
				object->SetTranslate(entry.transform.position);
				object->SetRotate(entry.transform.rotation);
				object->SetScale(entry.transform.scale);
				object->SetCamera(camera_); object->Update();
				objects.push_back({ std::move(object), entry.visible });
			}
			objects_ = std::move(objects); markers_ = std::move(data.markers); loadedPath_ = filePath;
			std::snprintf(pathInput_, sizeof(pathInput_), "%s", filePath.c_str());
			message_ = "シーン配置を読み込みました: " + filePath;
			return true;
		} catch (const std::exception& e) { message_ = e.what(); return false; }
	}
	void SceneLayout::Clear() { objects_.clear(); markers_.clear(); loadedPath_.clear(); }
	void SceneLayout::Update(Camera* camera) {
		camera_ = camera;
		for (auto& entry : objects_) { entry.object->SetCamera(camera); entry.object->Update(); }
	}
	void SceneLayout::Draw() { for (auto& entry : objects_) { if (entry.visible) { entry.object->Draw(dxCommon_); } } }
	void SceneLayout::ImGuiDebug() {
#ifdef USE_IMGUI
		ImGui::Begin("Blenderシーン配置");
		ImGui::InputText("配置JSON", pathInput_, sizeof(pathInput_));
		if (ImGui::Button("読込 / 再読込")) { const std::string path = pathInput_; Load(path); }
		ImGui::SameLine();
		if (ImGui::Button("サンプルを読込")) { Load("resources/layouts/sample_layout.json"); }
		ImGui::Text("モデル数: %zu / マーカー数: %zu", objects_.size(), markers_.size());
		ImGui::TextWrapped("%s", message_.c_str());
		ImGui::TextWrapped("Blenderで書き出した配置を読み込みます。失敗時は現在のシーン配置を残します。");
		ImGui::End();
#endif
	}
}
