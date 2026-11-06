#pragma once
#include <cstdint>

namespace TKM {

	//=============================================================
	// CollisionCategory
	// コライダーが何のオブジェクトなのかを表す種類。
	//
	// ゲーム側では数値を直接指定せず、
	// CollisionCategory::Playerのように名前で指定します。
	//=============================================================
	enum class CollisionCategory : uint32_t {
		None = 0,

		Player = 1u << 0,
		Plane = 1u << 1,
		Enemy = 1u << 2,
	};

	/// <summary>
	/// CollisionCategoryを内部計算用の数値へ変換します。
	/// ゲーム側から直接呼ぶ必要はありません。
	/// </summary>
	constexpr uint32_t ToCollisionBits(
		CollisionCategory category
	) {
		return static_cast<uint32_t>(category);
	}

} // namespace TKM
