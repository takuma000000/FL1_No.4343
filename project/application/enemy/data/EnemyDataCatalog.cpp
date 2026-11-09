#include "EnemyDataCatalog.h"

#include <utility>

#include "EnemyDataLoader.h"

bool EnemyDataCatalog::LoadDirectory(
	const std::filesystem::path& directoryPath,
	std::string& errorMessage
) {
	// 前回のエラー内容が残らないように初期化する。
	errorMessage.clear();

	// 指定されたパスが存在するか確認する。
	if (!std::filesystem::exists(directoryPath)) {
		errorMessage =
			"敵データフォルダが存在しません: " +
			directoryPath.string();

		return false;
	}

	// ファイルが指定された場合に、フォルダとして処理しないようにする。
	if (!std::filesystem::is_directory(directoryPath)) {
		errorMessage =
			"指定されたパスはフォルダではありません: " +
			directoryPath.string();

		return false;
	}

	try {
		// フォルダ内の項目を順番に確認する。
		for (
			const std::filesystem::directory_entry& entry :
			std::filesystem::directory_iterator(directoryPath)
		) {
			// サブフォルダなどは読み込み対象にしない。
			if (!entry.is_regular_file()) {
				continue;
			}

			// 現段階では、通常ファイルをすべてLoadFileへ渡す。
			if (!LoadFile(entry.path(), errorMessage)) {
				errorMessage =
					"敵データファイルの読み込みに失敗しました: " +
					entry.path().filename().string() +
					"\n" +
					errorMessage;

				return false;
			}
		}
	}
	catch (const std::filesystem::filesystem_error& exception) {
		errorMessage =
			"敵データフォルダの読み込み中にエラーが発生しました: " +
			std::string(exception.what());

		return false;
	}

	return true;
}

bool EnemyDataCatalog::LoadFile(
	const std::filesystem::path& filePath,
	std::string& errorMessage
) {
	// 前回のエラー内容が残らないように初期化する。
	errorMessage.clear();

	EnemyData loadedData{};

	// EnemyDataLoaderにJSONの解析と入力値検証を任せる。
	if (!EnemyDataLoader::LoadFromFile(
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
