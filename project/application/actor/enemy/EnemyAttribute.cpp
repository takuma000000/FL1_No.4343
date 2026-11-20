#include "EnemyAttribute.h"

std::optional<EnemyAttribute> EnemyAttributeFromString(
    std::string_view text
) {
    if (text == "None") {
        return EnemyAttribute::None;
    }

    if (text == "Natural") {
        return EnemyAttribute::Natural;
    }

    // 未知の文字列を特定の属性として扱わず、
    // 呼び出し側でエラー処理できるようにする。
    return std::nullopt;
}

std::string_view EnemyAttributeToString(
    EnemyAttribute attribute
) {
    switch (attribute) {
    case EnemyAttribute::None:
        return "None";

    case EnemyAttribute::Natural:
        return "Natural";

    default:
        return "Unknown";
    }
}
