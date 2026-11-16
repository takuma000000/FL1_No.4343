#include "ParticleEditor.h"
#include "ParticleEffects.h"
#include "ParticleManager.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#ifdef USE_IMGUI
#include "imgui.h"
#endif

namespace TKM {
	void ParticleEditor::Initialize() {
		std::snprintf(name_.data(), name_.size(), "%s", "newEffect");
		RefreshFiles();
		if (ParticleEffectStore::Exists("sampleExplosion")) { Load("sampleExplosion"); }
	}
	void ParticleEditor::RefreshFiles() {
		files_ = ParticleEffectStore::List();
		textures_.clear();
		std::error_code error;
		std::filesystem::recursive_directory_iterator it("resources/texture", error), end;
		while (!error && it != end) {
			if (it->is_regular_file(error)) {
				auto ext = it->path().extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				if (ext == ".png" || ext == ".dds" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") { textures_.push_back(it->path().generic_string()); }
			}
			it.increment(error);
		}
		std::sort(textures_.begin(), textures_.end());
	}
	void ParticleEditor::Restart() {
		auto* effects = ParticleEffects::GetInstance();
		playing_ = effects->Preview(settings_, position_);
		paused_ = false;
		if (!playing_) { message_ = effects->GetLastError(); }
	}
	void ParticleEditor::Load(const std::string& name) {
		ParticleEffectSettings loaded;
		if (!ParticleEffectStore::Load(name, loaded, message_)) { return; }
		settings_ = loaded;
		std::snprintf(name_.data(), name_.size(), "%s", name.c_str());
		selected_ = name;
		dirty_ = false; confirmOverwrite_ = false;
		ParticleEffects::GetInstance()->StopPreview();
		playing_ = false; paused_ = false;
		message_ = "読み込みました。再生ボタンで確認できます。";
	}
	void ParticleEditor::Save() {
		if (ParticleEffectStore::Save(name_.data(), settings_, message_)) {
			dirty_ = false; selected_ = name_.data();
			// 同じ実行中にゲームへ戻っても、保存した最新版を使う。
			ParticleEffects::GetInstance()->Reload(name_.data());
			RefreshFiles(); message_ = "保存しました: " + std::string(ParticleEffectStore::kDirectory) + "/" + name_.data() + ".json";
		}
		confirmOverwrite_ = false;
	}
	void ParticleEditor::ImGuiDebug() {
#ifdef USE_IMGUI
		ImGui::Begin("パーティクルエディタ");
		ImGui::TextUnformatted("名前を付けて保存すると、ゲームでも同じエフェクトを使えます。");
		if (dirty_) { ImGui::TextUnformatted("* 未保存の変更があります"); }
		bool changed = ImGui::InputText("エフェクト名", name_.data(), name_.size());
		ImGui::TextDisabled("名前: 半角英数字・_・-（1～64文字）");
		if (ImGui::Button("新規")) {
			if (dirty_) { confirmNew_ = true; } else { settings_ = {}; std::snprintf(name_.data(), name_.size(), "%s", "newEffect"); changed = true; }
		}
		ImGui::SameLine();
		if (ImGui::Button("一覧を更新")) { RefreshFiles(); }
		if (ImGui::BeginCombo("保存済み", selected_.c_str())) {
			for (const auto& file : files_) { if (ImGui::Selectable(file.c_str(), selected_ == file)) { selected_ = file; } }
			ImGui::EndCombo();
		}
		if (ImGui::Button("選択した設定を読み込む") && !selected_.empty()) {
			if (dirty_) { pendingLoad_ = selected_; } else { Load(selected_); }
		}
		if (confirmNew_ || !pendingLoad_.empty()) {
			ImGui::TextUnformatted("未保存の変更を破棄しますか？");
			if (ImGui::Button("破棄して進む")) {
				if (confirmNew_) { settings_ = {}; std::snprintf(name_.data(), name_.size(), "%s", "newEffect"); changed = true; }
				else { const auto name = pendingLoad_; Load(name); }
				confirmNew_ = false; pendingLoad_.clear();
			}
			ImGui::SameLine();
			if (ImGui::Button("キャンセル")) { confirmNew_ = false; pendingLoad_.clear(); }
		}
		ImGui::SeparatorText("再生");
		if (ImGui::Button("再生 / もう一度発生")) { Restart(); }
		ImGui::SameLine();
		if (ImGui::Button(paused_ ? "再開" : "一時停止")) { paused_ = !paused_; }
		ImGui::SameLine();
		if (ImGui::Button("停止・粒子を消す")) { ParticleEffects::GetInstance()->StopPreview(); playing_ = false; paused_ = false; }
		ImGui::Text("生存粒子: %zu / グループ上限512", ParticleManager::GetInstance()->GetActiveParticleCount());
		// プレビュー位置は保存しない。ゲームではPlayに渡した座標が使われる。
		bool previewChanged = ImGui::DragFloat3("プレビュー位置", &position_.x, 0.1f);
		ImGui::SeparatorText("発生");
		changed |= ImGui::Checkbox("連続発生（OFFなら一回だけ）", &settings_.loop);
		changed |= ImGui::Checkbox("前の粒子を寿命まで残す", &settings_.keepParticles);
		ImGui::TextDisabled("ON: 重ねて発生（炎・煙向け） / OFF: 次の発生時に消去");
		ImGui::TextDisabled("上限512粒子に達すると、新規生成を見送ります。");
		int count = static_cast<int>(settings_.count);
		if (ImGui::DragInt("一回の発生数", &count, 1, 1, 512)) { settings_.count = static_cast<uint32_t>(std::clamp(count, 1, 512)); changed = true; }
		changed |= ImGui::DragFloat("発生間隔（秒）", &settings_.interval, 0.01f, 0.01f, 60.0f);
		changed |= ImGui::DragFloat3("発生範囲（各軸の±）", &settings_.spawnExtent.x, 0.01f, 0.0f, 1000.0f);
		ImGui::SeparatorText("動き・寿命");
		changed |= ImGui::DragFloat3("初速度 最小", &settings_.velocityMin.x, 0.1f, -1000.0f, 1000.0f);
		changed |= ImGui::DragFloat3("初速度 最大", &settings_.velocityMax.x, 0.1f, -1000.0f, 1000.0f);
		changed |= ImGui::DragFloat3("重力 / 加速度", &settings_.gravity.x, 0.1f, -1000.0f, 1000.0f);
		changed |= ImGui::DragFloat("寿命 最小（秒）", &settings_.lifeMin, 0.01f, 0.01f, 60.0f);
		changed |= ImGui::DragFloat("寿命 最大（秒）", &settings_.lifeMax, 0.01f, 0.01f, 60.0f);
		ImGui::SeparatorText("見た目");
		const char* shapes[] = { "通常（板）", "リング", "シリンダー", "リボン（板）" };
		changed |= ImGui::Combo("形状", &settings_.shape, shapes, IM_ARRAYSIZE(shapes));
		if (ImGui::BeginCombo("テクスチャ", settings_.texture.c_str())) {
			for (const auto& path : textures_) { if (ImGui::Selectable(path.c_str(), path == settings_.texture)) { settings_.texture = path; changed = true; } }
			ImGui::EndCombo();
		}
		changed |= ImGui::DragFloat("開始サイズ", &settings_.startSize, 0.01f, 0.0f, 1000.0f);
		changed |= ImGui::DragFloat("終了サイズ", &settings_.endSize, 0.01f, 0.0f, 1000.0f);
		changed |= ImGui::ColorEdit4("開始色", &settings_.startColor.x);
		changed |= ImGui::ColorEdit4("終了色", &settings_.endColor.x);
		ImGui::TextDisabled("色・サイズは寿命に沿って変化。ブレンドは既存の加算合成です。");
		if (changed) { dirty_ = true; confirmOverwrite_ = false; }
		if ((changed || previewChanged) && playing_) { Restart(); }
		ImGui::TextDisabled("再生・調整時も「前の粒子を寿命まで残す」に従います。形状・テクスチャ変更時は消去します。");
		ImGui::SeparatorText("保存");
		if (ImGui::Button("保存")) {
			if (ParticleEffectStore::Exists(name_.data())) { confirmOverwrite_ = true; } else { Save(); }
		}
		if (confirmOverwrite_) {
			ImGui::Text("%s.json を上書きしますか？", name_.data());
			if (ImGui::Button("上書き保存")) { Save(); }
			ImGui::SameLine();
			if (ImGui::Button("上書きをやめる")) { confirmOverwrite_ = false; }
		}
		if (!message_.empty()) { ImGui::TextWrapped("%s", message_.c_str()); }
		ImGui::TextWrapped("シーン移動前に保存してください。未保存の編集はシーン終了時に失われます。");
		ImGui::End();
#endif
	}
}
