#include "CollisionManager.h"
#include "LineRenderer.h"
#include "Object3d.h"

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
		SizeGetter sizeGetter
	) {
		if (id.empty() || !positionGetter || !sizeGetter) {
			return nullptr;
		}

		// 同じ名前が登録済みなら追加しない
		if (entries_.contains(id)) {
			return nullptr;
		}

		const Vector3 position = positionGetter();
		const Vector3 size = sizeGetter();

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
				std::move(positionGetter),
				std::move(sizeGetter)
			}
		);

		return colliderPointer;
	}

	AABBCollider* CollisionManager::RegisterAABB(
		const std::string& id,
		const Object3d* object
	) {
		// 1. Object3dが存在しない場合は登録できない
		if (!object) {
			return nullptr;
		}

		// 2. Object3dから中心とサイズを自動取得する形で登録する
		return RegisterAABB(
			id,
			[object]() {
				return object->GetWorldBoundsCenter();
			},
			[object]() {
				return object->GetWorldBoundsSize();
			}
		);
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

		// デバッグ表示用の接触一覧からも削除する
		collidingColliders_.erase(collider);

		entries_.erase(iterator);
	}

	void CollisionManager::Update() {
		UpdateColliderTransforms();
		CheckAllCollisions();
	}

	void CollisionManager::Clear() {
		previousPairs_.clear();
		collidingColliders_.clear();
		entries_.clear();
	}

	void CollisionManager::DrawDebug(
		const Matrix4x4& viewProjection
	) {
		auto* lineRenderer =
			LineRenderer::GetInstance();

		// 1. 前フレームに登録したラインを消去する
		lineRenderer->BeginFrame();

		// 2. 登録されているすべてのAABBをラインへ追加する
		for (const auto& [id, entry] : entries_) {
			if (!entry.collider_ || !entry.collider_->IsActive()) {
				continue;
			}

			const bool isColliding =
				collidingColliders_.contains(
					entry.collider_.get()
				);

			// 通常時は緑、接触中は赤で表示する
			const LineRenderer::Color color =
				isColliding
				? LineRenderer::Color{ 0.0f, 0.0f, 1.0f, 1.0f }
				: LineRenderer::Color{ 0.0f, 1.0f, 0.0f, 1.0f };

			lineRenderer->AddAABB(
				entry.collider_->GetCenter(),
				entry.collider_->GetSize(),
				color
			);
		}

		// 3. 追加したAABBラインをまとめて描画する
		lineRenderer->Draw(viewProjection);
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

	void CollisionManager::UpdateColliderTransforms() {
		for (auto& [id, entry] : entries_) {
			if (!entry.positionGetter_ || !entry.sizeGetter_) {
				continue;
			}

			entry.collider_->SetCenter(
				entry.positionGetter_()
			);

			entry.collider_->SetSize(
				entry.sizeGetter_()
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
		std::set<Collider*> currentCollidingColliders;

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
				currentCollidingColliders.insert(first);
				currentCollidingColliders.insert(second);

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
		collidingColliders_ =
			std::move(currentCollidingColliders);
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
		// 1. firstがsecondを判定対象にしているか確認する
		const bool firstTargetsSecond =
			first.IsCollisionTarget(second);

		// 2. secondがfirstを判定対象にしているか確認する
		const bool secondTargetsFirst =
			second.IsCollisionTarget(first);

		// 3. お互いが判定対象にしている場合だけ当たり判定を行う
		return firstTargetsSecond
			&& secondTargetsFirst;
	}

} // namespace TKM
