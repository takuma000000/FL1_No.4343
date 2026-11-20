#include "SceneLayout.h"
#include "Object3dCommon.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <map>
#ifdef USE_IMGUI
#include "imgui.h"
#endif
namespace TKM {
	void SceneLayout::Initialize(DirectXCommon* dxCommon, Camera* camera) {
		dxCommon_ = dxCommon; camera_ = camera; RefreshModels();
	}
	bool SceneLayout::Apply(const SceneLayoutData& data) {
		if (!dxCommon_) { message_ = "SceneLayout::Initializeを先に呼んでください"; return false; }
		try {
			auto copied = data;
			std::vector<std::unique_ptr<Object3d>> objects(data.objects.size());
			std::vector<int> reuse(data.objects.size(), -1);
			std::vector<bool> used(objects_.size(), false);
			for (size_t i = 0; i < data.objects.size(); ++i) {
				const auto& entry = data.objects[i];
				for (size_t j = 0; j < objects_.size(); ++j) {
					if (!used[j] && data_.objects[j].model == entry.model) { reuse[i] = static_cast<int>(j); used[j] = true; break; }
				}
				if (reuse[i] >= 0) { continue; }
				auto object = std::make_unique<Object3d>();
				object->Initialize(Object3dCommon::GetInstance(), dxCommon_);
				object->SetModel(entry.model);
				object->SetTranslate(entry.transform.position);
				object->SetRotate(entry.transform.rotation);
				object->SetScale(entry.transform.scale);
				object->SetCamera(camera_); object->Update();
				objects[i] = std::move(object);
			}
			for (size_t i = 0; i < reuse.size(); ++i) { if (reuse[i] >= 0) { objects[i] = std::move(objects_[reuse[i]]); } }
			objects_ = std::move(objects); data_ = std::move(copied);
			selected_ = -1; Update(camera_); return true;
		} catch (const std::exception& e) { message_ = e.what(); return false; }
	}
	void SceneLayout::Remember(const SceneLayoutData& data) {
		if (undo_.size() >= 64) { undo_.erase(undo_.begin()); }
		undo_.push_back(data); redo_.clear(); dirty_ = true;
	}
	bool SceneLayout::Load(const std::string& filePath) {
		SceneLayoutData data; std::string error;
		if (!LoadSceneLayoutData(filePath, data, error)) { message_ = error; return false; }
		auto previous = data_;
		if (!Apply(data)) { return false; }
		Remember(previous); dirty_ = false;
		std::snprintf(pathInput_, sizeof(pathInput_), "%s", filePath.c_str());
		message_ = "配置を読み込みました: " + filePath; return true;
	}
	bool SceneLayout::Save(const std::string& filePath) {
		const auto path = std::filesystem::path(std::u8string(filePath.begin(), filePath.end()));
		// Blender出力を上書きしないよう、編集済み配置と保存先を分離する。
		if (path.lexically_normal().generic_string().find("resources/blender_exports/") != std::string::npos) {
			message_ = "ゲーム側の配置はresources/layouts/へ保存してください"; return false;
		}
		std::string error;
		if (!SaveSceneLayoutData(filePath, data_, error)) { message_ = error; return false; }
		dirty_ = false; message_ = "配置を保存しました: " + filePath; return true;
	}
	std::string SceneLayout::UniqueName(const std::string& base) const {
		std::string name = base; size_t suffix = 1;
		auto used = [&](const std::string& value) {
			for (const auto& o : data_.objects) { if (o.name == value) { return true; } }
			for (const auto& m : data_.markers) { if (m.name == value) { return true; } }
			return false;
		};
		while (used(name)) { name = base + "_" + std::to_string(suffix++); }
		return name;
	}
	bool SceneLayout::AddModel(const std::string& model) {
		if (data_.objects.size() >= 4096) { message_ = "配置数の上限です"; return false; }
		std::string error;
		if (!ValidateSceneLayoutModel(model, error)) { message_ = error; return false; }
		auto next = data_;
		auto found = std::find(models_.begin(), models_.end(), model);
		const auto name = found == models_.end() ? std::filesystem::path(model).stem().string() : modelLabels_[static_cast<size_t>(found - models_.begin())];
		next.objects.push_back({ UniqueName(name), model, {}, true });
		auto previous = data_;
		if (!Apply(next)) { return false; }
		Remember(previous); selected_ = static_cast<int>(objects_.size()) - 1;
		message_ = "モデルを追加しました"; return true;
	}
	bool SceneLayout::Duplicate(size_t index) {
		if (index >= data_.objects.size() || data_.objects.size() >= 4096) { return false; }
		auto next = data_; auto entry = next.objects[index]; entry.name = UniqueName(entry.name);
		next.objects.push_back(entry); auto previous = data_;
		if (!Apply(next)) { return false; }
		Remember(previous); selected_ = static_cast<int>(objects_.size()) - 1; return true;
	}
	bool SceneLayout::Delete(size_t index) {
		if (index >= data_.objects.size()) { return false; }
		auto next = data_; next.objects.erase(next.objects.begin() + index); auto previous = data_;
		if (!Apply(next)) { return false; }
		Remember(previous); return true;
	}
	bool SceneLayout::Undo() {
		if (undo_.empty()) { return false; }
		auto previous = data_;
		if (!Apply(undo_.back())) { return false; }
		undo_.pop_back(); redo_.push_back(std::move(previous)); dirty_ = true; return true;
	}
	bool SceneLayout::Redo() {
		if (redo_.empty()) { return false; }
		auto previous = data_;
		if (!Apply(redo_.back())) { return false; }
		redo_.pop_back(); undo_.push_back(std::move(previous)); dirty_ = true; return true;
	}
	void SceneLayout::New() {
		auto previous = data_;
		if (Apply({})) { Remember(previous); message_ = "空の配置を作成しました"; }
	}
	void SceneLayout::Clear() {
		objects_.clear(); data_ = {}; undo_.clear(); redo_.clear(); selected_ = -1; dirty_ = false;
	}
	void SceneLayout::RefreshModels() {
		models_.clear(); modelLabels_.clear(); selectedModel_ = -1;
		try {
			for (const auto& entry : std::filesystem::recursive_directory_iterator("resources/obj")) {
				if (entry.is_regular_file() && entry.path().extension() == ".obj") {
					models_.push_back(entry.path().lexically_relative("resources/obj").generic_string());
				}
			}
			std::sort(models_.begin(), models_.end());
			std::map<std::string, std::string> names;
			if (std::filesystem::exists("resources/blender_exports")) {
				for (const auto& entry : std::filesystem::directory_iterator("resources/blender_exports")) {
					if (!entry.is_regular_file() || entry.path().extension() != ".json") { continue; }
					SceneLayoutData exported; std::string error;
					const auto utf8 = entry.path().u8string();
					if (LoadSceneLayoutData(std::string(utf8.begin(), utf8.end()), exported, error)) {
						for (const auto& object : exported.objects) { names.try_emplace(object.model, object.name); }
					}
				}
			}
			for (const auto& model : models_) { modelLabels_.push_back(names.count(model) ? names[model] : std::filesystem::path(model).stem().string()); }
		} catch (const std::exception& e) { message_ = e.what(); }
	}
	void SceneLayout::Update(Camera* camera) {
		camera_ = camera;
		for (size_t i = 0; i < objects_.size(); ++i) {
			auto& object = objects_[i]; const auto& t = data_.objects[i].transform;
			object->SetTranslate(t.position); object->SetRotate(t.rotation); object->SetScale(t.scale);
			object->SetCamera(camera); object->Update();
		}
	}
	void SceneLayout::Draw() {
		for (size_t i = 0; i < objects_.size(); ++i) { if (data_.objects[i].visible) { objects_[i]->Draw(dxCommon_); } }
	}
	void SceneLayout::ImGuiDebug() {
#ifdef USE_IMGUI
		ImGui::Begin("シーン配置エディタ");
		ImGui::TextUnformatted(dirty_ ? "未保存の変更あり" : "変更なし");
		ImGui::InputText("配置JSON", pathInput_, sizeof(pathInput_));
		if (ImGui::Button("新規")) { ImGui::OpenPopup("新規配置の確認"); }
		ImGui::SameLine();
		if (ImGui::Button("読込")) { ImGui::OpenPopup("配置読込の確認"); }
		ImGui::SameLine();
		if (ImGui::Button("保存")) { ImGui::OpenPopup("配置保存の確認"); }
		if (ImGui::BeginPopupModal("新規配置の確認", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::TextUnformatted("現在の配置を空にします。Undoで戻せます。");
			if (ImGui::Button("新規作成")) { New(); ImGui::CloseCurrentPopup(); }
			ImGui::SameLine(); if (ImGui::Button("キャンセル")) { ImGui::CloseCurrentPopup(); } ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal("配置読込の確認", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::TextUnformatted("現在の配置を置き換えます。Undoで戻せます。");
			if (ImGui::Button("読み込む")) { const std::string path = pathInput_; Load(path); ImGui::CloseCurrentPopup(); }
			ImGui::SameLine(); if (ImGui::Button("キャンセル")) { ImGui::CloseCurrentPopup(); } ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal("配置保存の確認", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::TextWrapped("保存先: %s\n既存ファイルがあれば上書きします。", pathInput_);
			if (ImGui::Button("保存する")) { Save(pathInput_); ImGui::CloseCurrentPopup(); }
			ImGui::SameLine(); if (ImGui::Button("キャンセル")) { ImGui::CloseCurrentPopup(); } ImGui::EndPopup();
		}
		ImGui::BeginDisabled(undo_.empty()); if (ImGui::Button("Undo")) { Undo(); } ImGui::EndDisabled();
		ImGui::SameLine(); ImGui::BeginDisabled(redo_.empty()); if (ImGui::Button("Redo")) { Redo(); } ImGui::EndDisabled();
		ImGui::Separator();
		ImGui::TextUnformatted("モデルを選択して追加（選択だけでは描画しません）");
		if (ImGui::Button("モデル一覧を更新")) { RefreshModels(); }
		ImGui::InputText("モデル検索", filter_, sizeof(filter_));
		if (ImGui::BeginListBox("モデル", ImVec2(-1, 140))) {
			for (size_t i = 0; i < models_.size(); ++i) {
				if (models_[i].find(filter_) == std::string::npos && modelLabels_[i].find(filter_) == std::string::npos) { continue; }
				ImGui::PushID(static_cast<int>(i));
				const auto label = modelLabels_[i] + " (" + models_[i] + ")";
				if (ImGui::Selectable(label.c_str(), selectedModel_ == static_cast<int>(i))) { selectedModel_ = static_cast<int>(i); }
				ImGui::PopID();
			} ImGui::EndListBox();
		}
		ImGui::BeginDisabled(selectedModel_ < 0);
		if (ImGui::Button("選択したモデルを追加")) { const auto model = models_[selectedModel_]; AddModel(model); }
		ImGui::EndDisabled();
		ImGui::Separator(); ImGui::Text("配置数: %zu", data_.objects.size());
		if (ImGui::BeginListBox("配置済み", ImVec2(-1, 140))) {
			for (size_t i = 0; i < data_.objects.size(); ++i) {
				ImGui::PushID(static_cast<int>(i));
				if (ImGui::Selectable(data_.objects[i].name.c_str(), selected_ == static_cast<int>(i))) { selected_ = static_cast<int>(i); }
				ImGui::PopID();
			} ImGui::EndListBox();
		}
		if (selected_ >= 0 && selected_ < static_cast<int>(data_.objects.size())) {
			auto& entry = data_.objects[selected_];
			ImGui::TextWrapped("モデル: %s", entry.model.c_str());
			// 各入力の編集開始時に記録し、ドラッグ全体をUndo一回分にする。
			auto recordEdit = [&](bool changed) {
				if (ImGui::IsItemActivated()) { editRecorded_ = false; }
				if (changed && !editRecorded_) { Remember(data_); editRecorded_ = true; }
			};
			char name[512]; std::snprintf(name, sizeof(name), "%s", entry.name.c_str());
			if (ImGui::InputText("名前", name, sizeof(name), ImGuiInputTextFlags_EnterReturnsTrue)) {
				bool used = name[0] == '\0';
				for (size_t i = 0; i < data_.objects.size(); ++i) { if (static_cast<int>(i) != selected_ && data_.objects[i].name == name) { used = true; } }
				for (const auto& m : data_.markers) { if (m.name == name) { used = true; } }
				if (!used && entry.name != name) { Remember(data_); entry.name = name; } else if (used) { message_ = "名前は空欄・重複にできません"; }
			}
			// 値をコピーしてから操作し、履歴の編集前データに変更が混ざらないようにする。
			auto position = entry.transform.position;
			bool changed = ImGui::DragFloat3("位置", &position.x, 0.1f); recordEdit(changed);
			if (changed && std::isfinite(position.x) && std::isfinite(position.y) && std::isfinite(position.z)) { entry.transform.position = position; dirty_ = true; }
			constexpr float degrees = 57.295779513f;
			Vector3 rotation{ entry.transform.rotation.x * degrees, entry.transform.rotation.y * degrees, entry.transform.rotation.z * degrees };
			changed = ImGui::DragFloat3("回転（度）", &rotation.x, 0.5f); recordEdit(changed);
			if (changed && std::isfinite(rotation.x) && std::isfinite(rotation.y) && std::isfinite(rotation.z)) { entry.transform.rotation = { rotation.x / degrees, rotation.y / degrees, rotation.z / degrees }; dirty_ = true; }
			auto scale = entry.transform.scale;
			changed = ImGui::DragFloat3("サイズ", &scale.x, 0.01f); recordEdit(changed);
			if (changed && std::isfinite(scale.x) && std::isfinite(scale.y) && std::isfinite(scale.z)) {
				for (float* v : { &scale.x, &scale.y, &scale.z }) { if (std::abs(*v) < 0.0001f) { *v = std::signbit(*v) ? -0.0001f : 0.0001f; } }
				entry.transform.scale = scale; dirty_ = true;
			}
			bool visible = entry.visible;
			if (ImGui::Checkbox("表示", &visible)) { Remember(data_); entry.visible = visible; }
			if (ImGui::Button("複製")) { Duplicate(selected_); }
			ImGui::SameLine(); if (ImGui::Button("削除")) { Delete(selected_); }
		}
		ImGui::TextWrapped("%s", message_.c_str());
		ImGui::End();
#endif
	}
}
