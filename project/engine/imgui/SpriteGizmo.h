#pragma once
#include "Sprite.h"

namespace TKM {

	//=============================================================
	// SpriteGizmoクラス
	// マウスによるスプライトの移動・拡縮・回転を管理するクラス。
	//=============================================================
	class SpriteGizmo {
	public:
		/// <summary>
		/// マウスがスプライトの表示範囲内にあるか確認します。
		/// </summary>
		bool IsMouseOver(const Sprite* sprite) const;

		/// <summary>
		/// 選択中スプライトのGizmoを更新・描画します。
		/// </summary>
		void Update(Sprite* sprite);

		/// <summary>
		/// 操作状態を解除します。
		/// </summary>
		void Clear();

	private:
		enum class Handle {
			None,
			Move,
			TopLeft,
			Top,
			TopRight,
			Right,
			BottomRight,
			Bottom,
			BottomLeft,
			Left,
			Rotate,
		};

		struct Geometry {
			Vector2 center_{};
			Vector2 corners_[4]{};
			Vector2 handles_[8]{};
			Vector2 rotateHandle_{};
		};

		Geometry CalculateGeometry(const Sprite& sprite) const;
		Handle HitTestHandle(const Geometry& geometry) const;
		void BeginOperation(Sprite* sprite, Handle handle);
		void ApplyOperation(Sprite* sprite);
		void Draw(const Sprite& sprite) const;

		Handle activeHandle_ = Handle::None;
		Sprite* activeSprite_ = nullptr;

		Vector2 startMouse_{};
		Vector2 startPosition_{};
		Vector2 startSize_{};
		Vector2 startCenter_{};
		Vector2 startAnchor_{};
		float startRotation_ = 0.0f;
	};

} // namespace TKM
