#pragma once

#include <filesystem>
#include <string>

#include "../EnemyData.h"

//=============================================================
// 敵データローダークラス
// JSONファイルからEnemyDataを読み込む。
//=============================================================
class EnemyDataLoader {
public:
	/// <summary>
	/// 指定されたJSONファイルから敵データを読み込む。
	/// </summary>
	/// <param name="filePath">読み込むJSONファイルのパス</param>
	/// <param name="outputData">読み込んだ敵データの格納先</param>
	/// <param name="errorMessage">失敗した場合のエラーメッセージ</param>
	/// <returns>読み込みに成功した場合はtrue</returns>
	static bool LoadFromFile(
		const std::filesystem::path& filePath,
		EnemyData& outputData,
		std::string& errorMessage
	);
};