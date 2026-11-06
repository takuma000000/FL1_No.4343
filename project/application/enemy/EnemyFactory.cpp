#include "EnemyFactory.h"

#include "Object3dCommon.h"

std::unique_ptr<Enemy> EnemyFactory::Create(
	EnemyType type,
	const Vector3& position,
	TKM::DirectXCommon* dxCommon,
	TKM::Camera* camera
) {
	EnemyData data{};

	// 現在登録済みのモデルがsphere.objのみのため、
	// 敵の見た目はスケールで区別する。
	switch (type) {
	case EnemyType::Normal:
		data = { "sphere.obj", 3, 5.0f, 1.0f, 2.0f, 1.0f, { 1.0f, 1.0f, 1.0f } };
		break;

	case EnemyType::Flying:
		data = { "sphere.obj", 4, 4.0f, 1.0f, 2.0f, 0.8f, { 0.8f, 0.8f, 0.8f } };
		break;

	case EnemyType::Shooter:
		data = { "sphere.obj", 3, 3.0f, 1.0f, 10.0f, 1.0f, { 1.0f, 1.0f, 1.0f } };
		break;

	case EnemyType::Tank:
		data = { "sphere.obj", 15, 1.5f, 3.0f, 2.5f, 1.8f, { 1.8f, 1.8f, 1.8f } };
		break;

	case EnemyType::Boss:
		data = { "sphere.obj", 100, 2.0f, 5.0f, 4.0f, 3.0f, { 3.0f, 3.0f, 3.0f } };
		break;

	default:
		// 未知の値が渡された場合も、通常敵として安全に生成する。
		type = EnemyType::Normal;
		data = { "sphere.obj", 3, 5.0f, 1.0f, 2.0f, 1.0f, { 1.0f, 1.0f, 1.0f } };
		break;
	}

	auto enemy = std::make_unique<Enemy>();
	enemy->Initialize(
		type,
		data,
		position,
		TKM::Object3dCommon::GetInstance(),
		dxCommon,
		camera
	);

	return enemy;
}
