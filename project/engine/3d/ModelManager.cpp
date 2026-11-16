#include "ModelManager.h"
#include "ModelCommon.h"
#include "Model.h"
#include "DirectXCommon.h"
#include <stdexcept>

namespace TKM {
	ModelManager* ModelManager::GetInstance() {
		static ModelManager instance;
		return &instance;
	}

	ModelManager::~ModelManager() = default;

	void ModelManager::Finalize() {}

	void ModelManager::Initialize(TKM::DirectXCommon* dxCommon) {
		dxCommon_ = dxCommon;
		// モデルが参照する共通設定を、マネージャーの寿命中保持する
		if (!modelCommon_) {
			modelCommon_ = std::make_unique<TKM::ModelCommon>();
		}
		modelCommon_->Initialize(dxCommon_);
	}

	void ModelManager::LoadModel(const std::string& filePath, TKM::DirectXCommon* dxCommon) {
		//読み込み済みモデルを検索
		if (models_.contains(filePath)) {
			//読み込み済みなら早期return
			return;
		}
		if (!dxCommon) {
			throw std::runtime_error("ModelManager must be initialized before loading models");
		}
		if (!modelCommon_) {
			Initialize(dxCommon);
		}
		//モデルの生成とファイル読み込み、初期化
		std::unique_ptr<TKM::Model> model = std::make_unique<TKM::Model>();
		model->Initialize(modelCommon_.get(), dxCommon, "resources/obj", filePath);
		//モデルをmapコンテナに格納する
		models_.insert(std::make_pair(filePath, std::move(model)));
	}

	TKM::Model* ModelManager::FindModel(const std::string& filePath) {
		// 未読み込みなら読み込み、同じモデルは共有して再利用する
		LoadModel(filePath, dxCommon_);
		return models_.at(filePath).get();
	}
}