#include "CollisionManager.h"

#include <algorithm>
#include <utility>

namespace TKM {

	CollisionManager* CollisionManager::GetInstance() {
		static CollisionManager instance;
		return &instance;
	}

	AABBCollider* CollisionManager::RegisterAABB(
		const std::string& id,
		PositionGetter positionGetter,
		const Vector3& size
	) {
		if (id.empty() || !positionGetter) {
			return nullptr;
		}

		// 同じ名前が登録済みなら追加しない
		if (entries_.contains(id)) {
			return nullptr;
		}

		const Vector3 position = positionGetter();

		auto collider =
			std::make_unique<AABBCollider>(
				position,
				size
			);

		// 識別名を初期タグとして設定する
		collider->SetTag(id);

		AABBCollider* colliderPointer =
			collider.get();

		entries_.emplace(
			id,
			Entry{
				std::move(collider),
				std::move(positionGetter)
			}
		);

		return colliderPointer;
	}

	void CollisionManager::Unregister(
		const std::string& id
	) {
		const auto iterator = entries_.find(id);

		if (iterator == entries_.end()) {
			return;
		}

		Collider* collider =
			iterator->second.collider_.get();

		// 削除するColliderを含む衝突ペアを削除する
		std::erase_if(
			previousPairs_,
			[collider](const CollisionPair& pair) {
				return pair.first_ == collider
					|| pair.second_ == collider;
			}
		);

		entries_.erase(iterator);
	}

	void CollisionManager::Update() {
		UpdateColliderPositions();
		CheckAllCollisions();
	}

	void CollisionManager::Clear() {
		previousPairs_.clear();
		entries_.clear();
	}

	AABBCollider* CollisionManager::GetAABBCollider(
		const std::string& id
	) {
		const auto iterator = entries_.find(id);

		if (iterator == entries_.end()) {
			return nullptr;
		}

		return iterator->second.collider_.get();
	}

	void CollisionManager::UpdateColliderPositions() {
		for (auto& [id, entry] : entries_) {
			if (!entry.positionGetter_) {
				continue;
			}

			entry.collider_->SetCenter(
				entry.positionGetter_()
			);
		}
	}

	void CollisionManager::CheckAllCollisions() {
		std::vector<Collider*> colliders;
		colliders.reserve(entries_.size());

		for (auto& [id, entry] : entries_) {
			if (entry.collider_) {
				colliders.push_back(
					entry.collider_.get()
				);
			}
		}

		std::set<CollisionPair> currentPairs;

		for (
			size_t firstIndex = 0;
			firstIndex < colliders.size();
			++firstIndex
			) {
			Collider* first = colliders[firstIndex];

			if (!first || !first->IsActive()) {
				continue;
			}

			for (
				size_t secondIndex = firstIndex + 1;
				secondIndex < colliders.size();
				++secondIndex
				) {
				Collider* second =
					colliders[secondIndex];

				if (!second || !second->IsActive()) {
					continue;
				}

				if (!CanCollide(*first, *second)) {
					continue;
				}

				if (!first->IsColliding(*second)) {
					continue;
				}

				const CollisionPair pair =
					MakePair(first, second);

				currentPairs.insert(pair);

				if (previousPairs_.contains(pair)) {
					first->NotifyCollisionStay(second);
					second->NotifyCollisionStay(first);
				} else {
					first->NotifyCollisionEnter(second);
					second->NotifyCollisionEnter(first);
				}
			}
		}

		for (const CollisionPair& pair : previousPairs_) {
			if (!currentPairs.contains(pair)) {
				pair.first_->NotifyCollisionExit(
					pair.second_
				);

				pair.second_->NotifyCollisionExit(
					pair.first_
				);
			}
		}

		previousPairs_ = std::move(currentPairs);
	}

	bool CollisionManager::CollisionPair::operator<(
		const CollisionPair& other
		) const {
		const std::less<Collider*> less;

		if (first_ != other.first_) {
			return less(first_, other.first_);
		}

		return less(second_, other.second_);
	}

	CollisionManager::CollisionPair
		CollisionManager::MakePair(
			Collider* first,
			Collider* second
		) {
		const std::less<Collider*> less;

		if (less(second, first)) {
			std::swap(first, second);
		}

		return {
			first,
			second
		};
	}

	bool CollisionManager::CanCollide(
		const Collider& first,
		const Collider& second
	) {
		const bool firstAcceptsSecond =
			(first.GetMask() & second.GetLayer()) != 0u;

		const bool secondAcceptsFirst =
			(second.GetMask() & first.GetLayer()) != 0u;

		return firstAcceptsSecond
			&& secondAcceptsFirst;
	}

} // namespace TKM