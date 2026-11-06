#include "EnemyDataCatalog.h"

#include <utility>

#include "EnemyDataLoader.h"

bool EnemyDataCatalog::LoadFile(
	const std::filesystem::path& filePath,
	std::string& errorMessage
) {
	// 前回のエラー内容が残らないように初期化する。
	errorMessage.clear();

	EnemyData loadedData{};

	// EnemyDataLoaderにJSONの解析と入力値検証を任せる。
	if (
		!EnemyDataLoader::LoadFromFile(
			filePath,
			loadedData,
			errorMessage
		)
		) {
		return false;
	}

	// 同じIDを上書きすると設定ミスに気づけないため、
	// 重複している場合は登録を失敗させる。
	if (Contains(loadedData.id)) {
		errorMessage =
			"同じ敵IDがすでに登録されています: " +
			loadedData.id;

		return false;
	}

	// loadedDataを移動する前に、検索キーとして使うIDを保存する。
	const std::string id = loadedData.id;

	data_.emplace(
		id,
		std::move(loadedData)
	);

	return true;
}

const EnemyData* EnemyDataCatalog::FindById(
	const std::string& id
) const {
	const auto iterator = data_.find(id);

	if (iterator == data_.end()) {
		return nullptr;
	}

	return &iterator->second;
}

bool EnemyDataCatalog::Contains(
	const std::string& id
) const {
	return data_.contains(id);
}

std::size_t EnemyDataCatalog::GetDataCount() const {
	return data_.size();
}

void EnemyDataCatalog::Clear() {
	data_.clear();
}