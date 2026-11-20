#include "ParticleEmitter.h"
#include "DeltaTimeManager.h"

void ParticleEmitter::Initialize(std::string name, Vector3 pos){
	this->name_ = name;

	emitter_.count_ = 1;           // 毎フレーム1個出す
	emitter_.frequency_ = 0.0f;    // 0なら常時Emit
	emitter_.frequencyTime_ = 0.0f;  // 経過時間初期化
	emitter_.transform_.translate_ = pos; // エミッター位置設定
	emitter_.transform_.rotate_ = { 0.0f,0.0f,0.0f }; // 回転は0固定
	emitter_.transform_.scale_ = { 1.0f,1.0f,1.0f }; // スケールは1固定
}

void ParticleEmitter::Emit(){
	TKM::ParticleManager::GetInstance()->Emit(name_, emitter_.transform_.translate_, emitter_.count_); // パーティクル発生
}

void ParticleEmitter::Update() {
	// frequency==0なら毎フレーム Emit（常時噴射）
	if (emitter_.frequency_ <= 0.0f) {
		Emit(); // パーティクル発生
		return;
	}

	// 共通のDeltaTimeを取得
	const float deltaTime =
		TKM::DeltaTimeManager::GetInstance()->GetDeltaTime();

	// 発生間隔の経過時間を進める
	emitter_.frequencyTime_ += deltaTime;

	if (emitter_.frequencyTime_ >= emitter_.frequency_) {
		emitter_.frequencyTime_ -= emitter_.frequency_;
		Emit();
	}
}