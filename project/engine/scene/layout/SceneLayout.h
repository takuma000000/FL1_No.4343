#pragma once
#include "SceneLayoutData.h"
#include "Object3d.h"
#include <memory>
namespace TKM {
	// シーン配置の所有・描画。ゲームのPlayer/Enemyを作り直さない。
	class SceneLayout {
	public:
		void Initialize(DirectXCommon* dxCommon, Camera* camera);
		bool Load(const std::string& filePath);
		void Clear();
		void Update(Camera* camera);
		void Draw();
		void ImGuiDebug();
		size_t GetObjectCount() const { return objects_.size(); }
		const std::vector<SceneLayoutMarker>& GetMarkers() const { return markers_; }
		const std::string& GetMessage() const { return message_; }
	private:
		struct Object { std::unique_ptr<Object3d> object; bool visible; };
		DirectXCommon* dxCommon_ = nullptr;
		Camera* camera_ = nullptr;
		std::vector<Object> objects_;
		std::vector<SceneLayoutMarker> markers_;
		std::string loadedPath_, message_;
		char pathInput_[512] = "resources/layouts/layout.json";
	};
}
