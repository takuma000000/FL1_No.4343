#pragma once

#include <optional>
#include <string_view>

//=============================================================
// 敵の種類（現状は仮の名前）
// 敵の種類を定義するための列挙型
//=============================================================
enum class EnemyType {
	Normal,
	Flying,
	Shooter,
	Tank,
	Boss,
};

//==============================================================
// EnemyTypeの文字列変換関数
//==============================================================

/// <summary>
/// 文字列をEnemyTypeに変換する
/// 変換できない文字列の場合はstd::nulloptを返す
/// </summary>
/// <param name="text"> 変換する文字列 </param>
std::optional<EnemyType> EnemyTypeFromString(
	std::string_view text
);

/// <summary>
/// EnemyTypeを文字列に変換する
/// 未定義の値の場合は"Unknown"を返す
/// </summary>
/// <param name="type"> 変換するEnemyType </param>
std::string_view EnemyTypeToString(
	EnemyType type
);