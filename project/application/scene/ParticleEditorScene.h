#pragma once
#include "BaseScene.h"
#include "ParticleEditor.h"

class ParticleEditorScene : public TKM::BaseScene {
public:
	ParticleEditorScene(TKM::DirectXCommon* dxCommon, TKM::SrvManager* srvManager)
		: dxCommon_(dxCommon), srvManager_(srvManager) {}
	const char* GetSceneName() const override { return "PARTICLE_EDITOR"; }
	void Initialize() override;
	void Finalize() override;
	void Update() override;
	void Draw() override { Draw3D(); }
	void Draw3D() override;
	void DrawBack() override {}
	void DrawSprite() override {}
private:
	TKM::DirectXCommon* dxCommon_;
	TKM::SrvManager* srvManager_;
	TKM::ParticleEditor editor_;
	bool showGrid_ = true;
};
