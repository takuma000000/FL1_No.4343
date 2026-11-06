#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "../EnemyData.h"

//=============================================================
// 敵データカタログクラス
// JSONから読み込んだEnemyDataごとにIDを保存する
//=============================================================
class EnemyDataCatalog {
public:
	/// <summary>
	/// 指定された敵JSONを読み込み、カタログへ登録する。
	/// </summary>
	/// <param name="filePath">敵JSONファイルのパス</param>
	/// <param name="errorMessage">失敗した場合のエラーメッセージ</param>
	/// <returns>読み込みと登録に成功した場合はtrue</returns>
	bool LoadFile(
		const std::filesystem::path& filePath,
		std::string& errorMessage
	);

	/// <summary>
	/// 指定されたIDの敵データを取得する。
	/// </summary>
	/// <param name="id">検索する敵ID</param>
	/// <returns>
	/// 見つかった場合はEnemyDataへのポインタ、
	/// 見つからない場合はnullptr
	/// </returns>
	const EnemyData* FindById(
		const std::string& id
	) const;

	/// <summary>
	/// 指定されたIDが登録済みか確認する。
	/// </summary>
	bool Contains(
		const std::string& id
	) const;

	/// <summary>
	/// 登録されている敵データの数を取得する。
	/// </summary>
	std::size_t GetDataCount() const;

	/// <summary>
	/// 登録されている敵データをすべて削除する。
	/// </summary>
	void Clear();

private:
	// 敵の固有IDと、対応する設定データを保持する。
	std::unordered_map<std::string, EnemyData> data_;

};