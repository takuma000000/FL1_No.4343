#include "SpriteGizmo.h"

#include <algorithm>
#include <cmath>

#ifdef USE_IMGUI
#include "imgui.h"
#endif

namespace {

	Vector2 RotateVector(const Vector2& value, float rotation) {
		const float cosine = std::cos(rotation);
		const float sine = std::sin(rotation);

		return {
			value.x * cosine - value.y * sine,
			value.x * sine + value.y * cosine
		};
	}

	Vector2 Add(const Vector2& first, const Vector2& second) {
		return { first.x + second.x, first.y + second.y };
	}

	Vector2 Subtract(const Vector2& first, const Vector2& second) {
		return { first.x - second.x, first.y - second.y };
	}

	float DistanceSquared(const Vector2& first, const Vector2& second) {
		const float x = first.x - second.x;
		const float y = first.y - second.y;
		return x * x + y * y;
	}

}

namespace TKM {

	SpriteGizmo::Geometry SpriteGizmo::CalculateGeometry(
		const Sprite& sprite
	) const {
		Geometry geometry{};

		const Vector2& position = sprite.GetPosition();
		const Vector2& size = sprite.GetSize();
		const Vector2& anchor = sprite.GetAnchorPoint();
		const float rotation = sprite.GetRotation();

		const Vector2 centerOffset = {
			(0.5f - anchor.x) * size.x,
			(0.5f - anchor.y) * size.y
		};

		geometry.center_ = Add(
			position,
			RotateVector(centerOffset, rotation)
		);

		const Vector2 halfSize = {
			size.x * 0.5f,
			size.y * 0.5f
		};

		const Vector2 localCorners[4] = {
			{ -halfSize.x, -halfSize.y },
			{ halfSize.x, -halfSize.y },
			{ halfSize.x, halfSize.y },
			{ -halfSize.x, halfSize.y }
		};

		for (int index = 0; index < 4; ++index) {
			geometry.corners_[index] = Add(
				geometry.center_,
				RotateVector(localCorners[index], rotation)
			);
		}

		// 左上、上、右上、右、右下、下、左下、左
		geometry.handles_[0] = geometry.corners_[0];
		geometry.handles_[1] = {
			(geometry.corners_[0].x + geometry.corners_[1].x) * 0.5f,
			(geometry.corners_[0].y + geometry.corners_[1].y) * 0.5f
		};
		geometry.handles_[2] = geometry.corners_[1];
		geometry.handles_[3] = {
			(geometry.corners_[1].x + geometry.corners_[2].x) * 0.5f,
			(geometry.corners_[1].y + geometry.corners_[2].y) * 0.5f
		};
		geometry.handles_[4] = geometry.corners_[2];
		geometry.handles_[5] = {
			(geometry.corners_[2].x + geometry.corners_[3].x) * 0.5f,
			(geometry.corners_[2].y + geometry.corners_[3].y) * 0.5f
		};
		geometry.handles_[6] = geometry.corners_[3];
		geometry.handles_[7] = {
			(geometry.corners_[3].x + geometry.corners_[0].x) * 0.5f,
			(geometry.corners_[3].y + geometry.corners_[0].y) * 0.5f
		};

		// 上辺の中央から30px離れた位置へ回転ハンドルを置く
		geometry.rotateHandle_ = Add(
			geometry.handles_[1],
			RotateVector({ 0.0f, -30.0f }, rotation)
		);

		return geometry;
	}

	bool SpriteGizmo::IsMouseOver(const Sprite* sprite) const {
#ifdef USE_IMGUI
		if (!sprite) {
			return false;
		}

		const ImVec2 mouse = ImGui::GetMousePos();
		const Geometry geometry = CalculateGeometry(*sprite);
		const Vector2 relative = {
			mouse.x - geometry.center_.x,
			mouse.y - geometry.center_.y
		};

		const Vector2 local =
			RotateVector(relative, -sprite->GetRotation());

		const Vector2& size = sprite->GetSize();
		return std::abs(local.x) <= size.x * 0.5f
			&& std::abs(local.y) <= size.y * 0.5f;
#else
		return false;
#endif
	}

	SpriteGizmo::Handle SpriteGizmo::HitTestHandle(
		const Geometry& geometry
	) const {
#ifdef USE_IMGUI
		const ImVec2 mousePosition = ImGui::GetMousePos();
		const Vector2 mouse = { mousePosition.x, mousePosition.y };
		constexpr float kHitRadius = 10.0f;
		constexpr float kHitRadiusSquared = kHitRadius * kHitRadius;

		if (DistanceSquared(mouse, geometry.rotateHandle_)
			<= kHitRadiusSquared) {
			return Handle::Rotate;
		}

		const Handle handles[8] = {
			Handle::TopLeft,
			Handle::Top,
			Handle::TopRight,
			Handle::Right,
			Handle::BottomRight,
			Handle::Bottom,
			Handle::BottomLeft,
			Handle::Left
		};

		for (int index = 0; index < 8; ++index) {
			if (DistanceSquared(mouse, geometry.handles_[index])
				<= kHitRadiusSquared) {
				return handles[index];
			}
		}
#endif

		return Handle::None;
	}

	void SpriteGizmo::BeginOperation(
		Sprite* sprite,
		Handle handle
	) {
#ifdef USE_IMGUI
		const ImVec2 mouse = ImGui::GetMousePos();
		activeSprite_ = sprite;
		activeHandle_ = handle;
		startMouse_ = { mouse.x, mouse.y };
		startPosition_ = sprite->GetPosition();
		startSize_ = sprite->GetSize();
		startAnchor_ = sprite->GetAnchorPoint();
		startRotation_ = sprite->GetRotation();
		startCenter_ = CalculateGeometry(*sprite).center_;
#endif
	}

	void SpriteGizmo::ApplyOperation(Sprite* sprite) {
#ifdef USE_IMGUI
		const ImVec2 mousePosition = ImGui::GetMousePos();
		const Vector2 mouse = { mousePosition.x, mousePosition.y };

		if (activeHandle_ == Handle::Move) {
			const Vector2 delta = Subtract(mouse, startMouse_);
			sprite->SetPosition(Add(startPosition_, delta));
			return;
		}

		if (activeHandle_ == Handle::Rotate) {
			const Vector2 direction = Subtract(mouse, startCenter_);
			const float rotation =
				std::atan2(direction.y, direction.x)
				+ 1.57079632679f;
			sprite->SetRotation(rotation);
			return;
		}

		const Vector2 mouseDelta = Subtract(mouse, startMouse_);
		const Vector2 localDelta =
			RotateVector(mouseDelta, -startRotation_);

		float horizontalDirection = 0.0f;
		float verticalDirection = 0.0f;

		switch (activeHandle_) {
		case Handle::TopLeft:
			horizontalDirection = -1.0f;
			verticalDirection = -1.0f;
			break;
		case Handle::Top:
			verticalDirection = -1.0f;
			break;
		case Handle::TopRight:
			horizontalDirection = 1.0f;
			verticalDirection = -1.0f;
			break;
		case Handle::Right:
			horizontalDirection = 1.0f;
			break;
		case Handle::BottomRight:
			horizontalDirection = 1.0f;
			verticalDirection = 1.0f;
			break;
		case Handle::Bottom:
			verticalDirection = 1.0f;
			break;
		case Handle::BottomLeft:
			horizontalDirection = -1.0f;
			verticalDirection = 1.0f;
			break;
		case Handle::Left:
			horizontalDirection = -1.0f;
			break;
		default:
			return;
		}

		Vector2 newSize = startSize_;
		if (horizontalDirection != 0.0f) {
			newSize.x = std::max(
				1.0f,
				startSize_.x
				+ localDelta.x * horizontalDirection
			);
		}

		if (verticalDirection != 0.0f) {
			newSize.y = std::max(
				1.0f,
				startSize_.y
				+ localDelta.y * verticalDirection
			);
		}

		sprite->SetSize(newSize);

		// ドラッグした辺に合わせて中心を移動し、反対側の辺を固定する
		const Vector2 localCenterDelta = {
			horizontalDirection != 0.0f
				? (newSize.x - startSize_.x)
					* horizontalDirection * 0.5f
				: 0.0f,
			verticalDirection != 0.0f
				? (newSize.y - startSize_.y)
					* verticalDirection * 0.5f
				: 0.0f
		};

		const Vector2 newCenter = Add(
			startCenter_,
			RotateVector(localCenterDelta, startRotation_)
		);

		// アンカーポイントを考慮して新しい位置を求める
		const Vector2 newCenterOffset = RotateVector(
			{
				(0.5f - startAnchor_.x) * newSize.x,
				(0.5f - startAnchor_.y) * newSize.y
			},
			startRotation_
		);

		sprite->SetPosition(
			Subtract(newCenter, newCenterOffset)
		);
#endif
	}

	void SpriteGizmo::Draw(const Sprite& sprite) const {
#ifdef USE_IMGUI
		const Geometry geometry = CalculateGeometry(sprite);
		// ゲーム画面より手前、ImGuiウィンドウより奥へGizmoを描画する
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();

		const ImU32 lineColor = IM_COL32(40, 130, 255, 255);
		const ImU32 handleFill = IM_COL32(255, 255, 255, 255);
		const ImU32 handleBorder = IM_COL32(40, 130, 255, 255);

		for (int index = 0; index < 4; ++index) {
			const Vector2& first = geometry.corners_[index];
			const Vector2& second = geometry.corners_[(index + 1) % 4];
			drawList->AddLine(
				{ first.x, first.y },
				{ second.x, second.y },
				lineColor,
				2.0f
			);
		}

		// 上辺と回転ハンドルをつなぐ線
		drawList->AddLine(
			{ geometry.handles_[1].x, geometry.handles_[1].y },
			{ geometry.rotateHandle_.x, geometry.rotateHandle_.y },
			lineColor,
			2.0f
		);

		for (const Vector2& handle : geometry.handles_) {
			drawList->AddCircleFilled(
				{ handle.x, handle.y },
				6.0f,
				handleFill
			);
			drawList->AddCircle(
				{ handle.x, handle.y },
				6.0f,
				handleBorder,
				0,
				2.0f
			);
		}

		drawList->AddCircleFilled(
			{ geometry.rotateHandle_.x, geometry.rotateHandle_.y },
			8.0f,
			handleFill
		);
		drawList->AddCircle(
			{ geometry.rotateHandle_.x, geometry.rotateHandle_.y },
			8.0f,
			handleBorder,
			0,
			2.0f
		);

		// 回転ハンドルの中心へ目印を描く
		drawList->AddText(
			{
				geometry.rotateHandle_.x - 4.0f,
				geometry.rotateHandle_.y - 7.0f
			},
			lineColor,
			"R"
		);
#endif
	}

	void SpriteGizmo::Update(Sprite* sprite) {
#ifdef USE_IMGUI
		if (!sprite) {
			Clear();
			return;
		}

		if (activeSprite_ && activeSprite_ != sprite) {
			Clear();
		}

		Draw(*sprite);

		if (activeHandle_ == Handle::None
			&& ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			&& !ImGui::GetIO().WantCaptureMouse) {
			const Geometry geometry = CalculateGeometry(*sprite);
			Handle handle = HitTestHandle(geometry);

			if (handle == Handle::None && IsMouseOver(sprite)) {
				handle = Handle::Move;
			}

			if (handle != Handle::None) {
				BeginOperation(sprite, handle);
			}
		}

		if (activeHandle_ != Handle::None) {
			ImGui::SetNextFrameWantCaptureMouse(true);

			if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				ApplyOperation(sprite);
			} else {
				Clear();
			}
		}
#else
		(void)sprite;
#endif
	}

	void SpriteGizmo::Clear() {
		activeHandle_ = Handle::None;
		activeSprite_ = nullptr;
	}

} // namespace TKM
