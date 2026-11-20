#pragma once
#include "SceneLayoutData.h"
#include "Object3d.h"
#include <memory>
namespace TKM {
	class SceneLayout {
	public:
		void Initialize(DirectXCommon* dxCommon, Camera* camera);
		bool Load(const std::string& filePath);
		bool Save(const std::string& filePath);
		bool AddModel(const std::string& model);
		bool Duplicate(size_t index);
		bool Delete(size_t index);
		bool Undo();
		bool Redo();
		void New();
		void Clear();
		void Update(Camera* camera);
		void Draw();
		void ImGuiDebug();
		size_t GetObjectCount() const { return objects_.size(); }
		const SceneLayoutData& GetData() const { return data_; }
		const std::vector<SceneLayoutMarker>& GetMarkers() const { return data_.markers; }
		const std::string& GetMessage() const { return message_; }
	private:
		bool Apply(const SceneLayoutData& data);
		void Remember(const SceneLayoutData& data);
		void RefreshModels();
		std::string UniqueName(const std::string& base) const;
		DirectXCommon* dxCommon_ = nullptr;
		Camera* camera_ = nullptr;
		std::vector<std::unique_ptr<Object3d>> objects_;
		SceneLayoutData data_;
		std::vector<SceneLayoutData> undo_, redo_;
		std::vector<std::string> models_, modelLabels_;
		int selected_ = -1, selectedModel_ = -1;
		bool dirty_ = false, editRecorded_ = false;
		std::string message_;
		char pathInput_[512] = "resources/layouts/scene.json";
		char filter_[128]{};
	};
}
