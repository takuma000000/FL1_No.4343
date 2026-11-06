#include "AABBCollider.h"

#include <cmath>

namespace TKM {

	AABBCollider::AABBCollider() {
		UpdateBounds();
	}

	AABBCollider::AABBCollider(
		const Vector3& center,
		const Vector3& size
	) {
		Set(center, size);
	}

	void AABBCollider::Set(
		const Vector3& center,
		const Vector3& size
	) {
		center_ = center;

		size_ = {
			std::abs(size.x),
			std::abs(size.y),
			std::abs(size.z)
		};

		UpdateBounds();
	}

	void AABBCollider::SetCenter(
		const Vector3& center
	) {
		center_ = center;
		UpdateBounds();
	}

	void AABBCollider::SetSize(
		const Vector3& size
	) {
		size_ = {
			std::abs(size.x),
			std::abs(size.y),
			std::abs(size.z)
		};

		UpdateBounds();
	}

	bool AABBCollider::IsColliding(
		const Collider& other
	) const {
		// 相手がAABBColliderか確認する
		const auto* otherAABB =
			dynamic_cast<const AABBCollider*>(&other);

		if (!otherAABB) {
			return false;
		}

		return bounds_.IsCollidingWithAABB(
			otherAABB->bounds_
		);
	}

	void AABBCollider::UpdateBounds() {
		bounds_.Set(center_, size_);
	}

} // namespace TKM