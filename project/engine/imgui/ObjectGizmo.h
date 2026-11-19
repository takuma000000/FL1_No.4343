#pragma once
#include "ObjectSave.h"
#include "Camera.h"

namespace TKM {
	class ObjectGizmo {
	public:
		enum class Operation { Translate, Rotate, Scale };
		void SetEnabled(bool enabled);
		void SetOperation(Operation operation) { operation_ = static_cast<int>(operation); }
		void ImGuiControls();
		bool Manipulate(ObjectSave::Values& values, const Camera& camera, int id);
		bool IsBusy() const;
		bool IsEnabled() const { return enabled_; }
		// 数値入力や通常のImGuiウィンドウの上では選択しない。
		bool CanPick() const;
		void Clear();
	private:
		bool enabled_ = false;
		int currentId_ = -1;
		int operation_ = 0; // 移動・回転・拡縮
		bool local_ = false, snap_ = false;
		float moveSnap_ = 1.0f, rotateSnap_ = 15.0f, scaleSnap_ = 0.1f;
	};
}
