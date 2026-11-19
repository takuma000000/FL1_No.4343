#pragma once
#include "MyMath.h"
#include <limits>

namespace TKM::ObjectGizmoMath {
	struct Ray { Vector3 origin{}, direction{}; float maxDistance = std::numeric_limits<float>::max(); };
	// mouseは描画領域左上からの座標。DirectXの深度範囲0～1で逆投影する。
	bool MakeRay(float mouseX, float mouseY, float width, float height, const Matrix4x4& viewProjection, Ray& out);
	bool HitBounds(const Ray& ray, const Vector3& center, const Vector3& size, float& distance);
	// エンジンの行ベクトル・Rx*Ry*Rz・ラジアン規約で分解する。
	// ミラー拡縮は元の符号を維持し、ゼロスケールやシアーは受け付けない。
	bool ReadRotation(const Matrix4x4& world, const Vector3& signedScale, Vector3& rotation);
}
