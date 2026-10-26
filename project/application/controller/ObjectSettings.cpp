#include "ObjectSettings.h"

void ObjectSettings::Initialize(
	TKM::Object3d* sphere,
	Player* player
) {
	// 保存先を設定
	objectSave_.SetSettingsPath("resources/data/GAME.json");

	// テスト用Sphereを保存対象へ登録
	if (sphere) {
		objectSave_.RegisterObject(
			"Sphere",
			[sphere]() {
				TKM::ObjectSave::Values values{};

				values.position_ = sphere->GetTranslate();
				values.rotation_ = sphere->GetRotate();
				values.scale_ = sphere->GetScale();
				values.visible_ = std::nullopt;

				return values;
			},
			[sphere](const TKM::ObjectSave::Values& values) {
				sphere->SetTranslate(values.position_);
				sphere->SetRotate(values.rotation_);
				sphere->SetScale(values.scale_);
			}
		);
	}

	// Playerを保存対象へ登録
	if (player) {
		objectSave_.RegisterObject(
			"Player",
			[player]() {
				TKM::ObjectSave::Values values{};

				values.position_ = player->GetPos();
				values.rotation_ = player->GetRotation();
				values.scale_ = player->GetScale();
				values.visible_ = player->IsVisible();

				return values;
			},
			[player](const TKM::ObjectSave::Values& values) {
				player->SetPos(values.position_);
				player->SetRotation(values.rotation_);
				player->SetScale(values.scale_);

				if (values.visible_.has_value()) {
					player->SetVisible(
						values.visible_.value()
					);
				}
			}
		);
	}

#ifdef USE_IMGUI

	// 編集画面へ保存機能を設定
	objectEditor_.SetObjectSave(&objectSave_);

	// Playerを編集対象へ登録
	if (player) {
		objectEditor_.RegisterObject(
			"Player",
			[player]() {
				player->ImGuiDebug();
			}
		);
	}

	// テスト用Sphereを編集対象へ登録
	if (sphere) {
		objectEditor_.RegisterObject(
			"Sphere",
			[sphere]() {
				sphere->ImGuiDebug();
			}
		);
	}

#endif

	// 全オブジェクトの登録後に保存値を読み込む
	objectSave_.LoadSettings();
}

void ObjectSettings::Finalize() {
	// オブジェクトへの参照を先に解除する
	objectEditor_.Clear();
	objectSave_.Clear();
}

void ObjectSettings::ImGuiDebug() {
#ifdef USE_IMGUI
	objectEditor_.ImGuiDebug();
#endif
}