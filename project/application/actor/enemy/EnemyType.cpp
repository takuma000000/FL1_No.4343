#include "EnemyType.h"

std::optional<EnemyType> EnemyTypeFromString(std::string_view text) {
    if (text == "Normal") {
        return EnemyType::Normal;
    }
    else if (text == "Flying") {
        return EnemyType::Flying;
    }
    else if (text == "Shooter") {
        return EnemyType::Shooter;
    }
    else if (text == "Tank") {
        return EnemyType::Tank;
    }
    else if (text == "Boss") {
        return EnemyType::Boss;
    }

    // 未知の文字列が渡された場合はstd::nulloptを返す
    // 変換失敗を表すstd::nulloptを返すことで、呼び出し側でエラー処理を行えるようにする。
    return std::nullopt;
}

std::string_view EnemyTypeToString(EnemyType type) {
    switch (type) {
    case EnemyType::Normal:
        return "Normal";

    case EnemyType::Flying:
        return "Flying";

    case EnemyType::Shooter:
        return "Shooter";

    case EnemyType::Tank:
        return "Tank";

    case EnemyType::Boss:
        return "Boss";

    default:
        // 未定義の値が渡された場合は"Unknown"を返す
        return "Unknown";
    }
}