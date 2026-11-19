#include "ObjectGizmoMath.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace TKM::ObjectGizmoMath {
	namespace {
		constexpr float kEpsilon = 0.00001f;
		bool Unproject(float x, float y, float z, const Matrix4x4& inverse, Vector3& out) {
			const float w = x * inverse.m[0][3] + y * inverse.m[1][3] + z * inverse.m[2][3] + inverse.m[3][3];
			if (!std::isfinite(w) || std::abs(w) < kEpsilon) { return false; }
			out = { (x * inverse.m[0][0] + y * inverse.m[1][0] + z * inverse.m[2][0] + inverse.m[3][0]) / w,
				(x * inverse.m[0][1] + y * inverse.m[1][1] + z * inverse.m[2][1] + inverse.m[3][1]) / w,
				(x * inverse.m[0][2] + y * inverse.m[1][2] + z * inverse.m[2][2] + inverse.m[3][2]) / w };
			return std::isfinite(out.x) && std::isfinite(out.y) && std::isfinite(out.z);
		}
	}
	bool MakeRay(float mouseX, float mouseY, float width, float height, const Matrix4x4& viewProjection, Ray& out) {
		if (width <= 0 || height <= 0 || !std::isfinite(mouseX) || !std::isfinite(mouseY)
			|| mouseX < 0 || mouseY < 0 || mouseX >= width || mouseY >= height) { return false; }
		auto vp = viewProjection;
		const auto inverse = MyMath::Inverse4x4(vp);
		const float x = mouseX / width * 2.0f - 1.0f, y = 1.0f - mouseY / height * 2.0f;
		Vector3 nearPoint{}, farPoint{};
		if (!Unproject(x, y, 0, inverse, nearPoint) || !Unproject(x, y, 1, inverse, farPoint)) { return false; }
		Vector3 direction = farPoint - nearPoint;
		const float length = std::sqrt(direction.Dot(direction));
		if (!std::isfinite(length) || length < kEpsilon) { return false; }
		out = { nearPoint, direction / length, length }; return true;
	}
	bool HitBounds(const Ray& ray, const Vector3& center, const Vector3& size, float& distance) {
		float nearValue = 0, farValue = ray.maxDistance;
		const float origin[] = { ray.origin.x, ray.origin.y, ray.origin.z };
		const float direction[] = { ray.direction.x, ray.direction.y, ray.direction.z };
		const float c[] = { center.x, center.y, center.z }, s[] = { size.x, size.y, size.z };
		for (int axis = 0; axis < 3; ++axis) {
			if (!std::isfinite(c[axis]) || !std::isfinite(s[axis]) || !std::isfinite(origin[axis]) || !std::isfinite(direction[axis])) { return false; }
			const float lo = c[axis] - std::abs(s[axis]) * 0.5f, hi = c[axis] + std::abs(s[axis]) * 0.5f;
			if (std::abs(direction[axis]) < kEpsilon) {
				if (origin[axis] < lo || origin[axis] > hi) { return false; }
			} else {
				float a = (lo - origin[axis]) / direction[axis], b = (hi - origin[axis]) / direction[axis];
				if (a > b) { std::swap(a, b); }
				nearValue = std::max(nearValue, a); farValue = std::min(farValue, b);
				if (nearValue > farValue) { return false; }
			}
		}
		distance = nearValue; return true;
	}
	bool ReadRotation(const Matrix4x4& world, const Vector3& signedScale, Vector3& rotation) {
		const float signs[] = { signedScale.x, signedScale.y, signedScale.z };
		float r[3][3]{};
		for (int i = 0; i < 3; ++i) {
			const float length = std::sqrt(world.m[i][0]*world.m[i][0] + world.m[i][1]*world.m[i][1] + world.m[i][2]*world.m[i][2]);
			if (!std::isfinite(length) || length < kEpsilon || signs[i] == 0) { return false; }
			for (int j = 0; j < 3; ++j) { r[i][j] = world.m[i][j] / std::copysign(length, signs[i]); }
		}
		for (int i = 0; i < 3; ++i) {
			const int j = (i+1)%3;
			if (std::abs(r[i][0]*r[j][0]+r[i][1]*r[j][1]+r[i][2]*r[j][2]) > 0.001f) { return false; }
		}
		const float cy = std::hypot(r[0][0], r[0][1]);
		rotation = { cy > kEpsilon ? std::atan2(r[1][2], r[2][2]) : std::atan2(-r[2][1], r[1][1]),
			std::atan2(-r[0][2], cy), cy > kEpsilon ? std::atan2(r[0][1], r[0][0]) : 0.0f };
		return true;
	}
}
