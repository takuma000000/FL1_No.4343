#include "CollisionManager.h"

#include <algorithm>
#include <functional>
#include <utility>

namespace TKM {

	CollisionManager* CollisionManager::GetInstance() {
		static CollisionManager instance;
		return &instance;
	}

	bool CollisionManager::Register(
		Collider* collider
	) {
		if (!collider) {
			return false;
		}

		// 同じコライダーが登録済みなら追加しない
		const auto iterator = std::find(
			colliders_.begin(),
			colliders_.end(),
			collider
		);

		if (iterator != colliders_.end()) {
			return false;
		}

		colliders_.push_back(collider);
		return true;
	}

	void CollisionManager::Unregister(
		Collider* collider
	) {
		if (!collider) {
			return;
		}

		// 登録一覧から削除する
		std::erase(colliders_, collider);

		// 削除対象を含む衝突ペアも削除する
		std::erase_if(
			previousPairs_,
			[collider](const CollisionPair& pair) {
				return pair.first_ == collider
					|| pair.second_ == collider;
			}
		);
	}

	void CollisionManager::CheckAllCollisions() {
		std::set<CollisionPair> currentPairs;

		for (
			size_t firstIndex = 0;
			firstIndex < colliders_.size();
			++firstIndex
			) {
			Collider* first = colliders_[firstIndex];

			if (!first || !first->IsActive()) {
				continue;
			}

			for (
				size_t secondIndex = firstIndex + 1;
				secondIndex < colliders_.size();
				++secondIndex
				) {
				Collider* second =
					colliders_[secondIndex];

				if (!second || !second->IsActive()) {
					continue;
				}

				// レイヤー設定上、判定対象でなければ飛ばす
				if (!CanCollide(*first, *second)) {
					continue;
				}

				// 実際に接触していなければ飛ばす
				if (!first->IsColliding(*second)) {
					continue;
				}

				const CollisionPair pair =
					MakePair(first, second);

				currentPairs.insert(pair);

				// 前フレームにも接触していた場合
				if (previousPairs_.contains(pair)) {
					first->NotifyCollisionStay(second);
					second->NotifyCollisionStay(first);
				}
				// 今フレームから接触した場合
				else {
					first->NotifyCollisionEnter(second);
					second->NotifyCollisionEnter(first);
				}
			}
		}

		// 前フレームには存在し、今フレームには存在しないペアを探す
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

	void CollisionManager::Clear() {
		previousPairs_.clear();
		colliders_.clear();
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

		// ポインタの並び順を統一する
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