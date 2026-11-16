#pragma once

#include <optional>
#include <string_view>

//=============================================================
// 敵の種類
// 敵の属性・弱点・耐性を定義するための列挙型
//=============================================================
enum class EnemyAttribute {
    None,
    Neutral,
};

//==============================================================
// EnemyAttributeの文字列変換関数
//==============================================================

/// <summary>
/// 文字列をEnemyAttributeへ変換する
/// 変換できない文字列の場合はstd::nulloptを返す
/// </summary>
/// <param name="text">変換する文字列</param>
std::optional<EnemyAttribute> EnemyAttributeFromString(
    std::string_view text
);

/// <summary>
/// EnemyAttributeを文字列に変換する
/// 未定義の値は"Unknown"を返す
/// </summary>
/// <param name="attribute">変換する属性</param>
std::string_view EnemyAttributeToString(
    EnemyAttribute attribute
);


