#pragma once
#include "Vector3.h"
#include <string>
#include <vector>
namespace TKM {
	struct SceneLayoutTransform {
		Vector3 position{}, rotation{}, scale{ 1, 1, 1 };
	};
	struct SceneLayoutObjectData { std::string name, model; SceneLayoutTransform transform; bool visible = true; };
	struct SceneLayoutMarker { std::string name, tag; SceneLayoutTransform transform; };
	struct SceneLayoutData { std::vector<SceneLayoutObjectData> objects; std::vector<SceneLayoutMarker> markers; };
	// GPUに触れる前に全データとOBJ/MTL/テクスチャの参照を検証する。
	// 失敗してもoutは変更しない。
	bool LoadSceneLayoutData(const std::string& filePath, SceneLayoutData& out, std::string& error);
	bool ValidateSceneLayoutModel(const std::string& model, std::string& error);
	bool SaveSceneLayoutData(const std::string& filePath, const SceneLayoutData& data, std::string& error);
}
