#include "SpriteEditor.h"

#include <algorithm>
#include <utility>

#ifdef USE_IMGUI
#include "imgui.h"
#endif

namespace TKM {

	void SpriteEditor::SetSpriteSave(SpriteSave* spriteSave) {
		spriteSave_ = spriteSave;
	}

	bool SpriteEditor::RegisterSprite(
		const std::string& name,
		Sprite* sprite,
		ImGuiCallback callback
	) {
		if (name.empty() || !sprite || !callback) {
			return false;
		}

		const auto iterator = std::find_if(
			entries_.begin(),
			entries_.end(),
			[&name](const Entry& entry) {
				return entry.name_ == name;
			}
		);

		if (iterator != entries_.end()) {
			return false;
		}

		entries_.push_back({ name, sprite, std::move(callback) });
		return true;
	}

	void SpriteEditor::UnregisterSprite(const std::string& name) {
		std::erase_if(
			entries_,
			[&name](const Entry& entry) {
				return entry.name_ == name;
			}
		);

		selectedIndex_ = 0;
	}

	void SpriteEditor::Clear() {
		entries_.clear();
		selectedIndex_ = 0;
		spriteSave_ = nullptr;
		spriteGizmo_.Clear();
	}

	void SpriteEditor::ImGuiDebug() {
#ifdef USE_IMGUI
		ImGui::Begin("スプライト編集");

		// ──────────────── スプライト保存 ───────────────
		if (spriteSave_) {
			if (ImGui::Button("全スプライトを保存")) {
				spriteSave_->SaveSettings();
			}

			ImGui::SameLine();

			if (ImGui::Button("最後の保存状態に戻す")) {
				spriteSave_->LoadSettings();
			}

			const std::string& message =
				spriteSave_->GetSettingsMessage();

			if (!message.empty()) {
				ImGui::TextWrapped("%s", message.c_str());
			}
		}

		// ──────────────── スプライト選択 ───────────────
		ImGui::Separator();
		const char* previewName = "None";

		if (selectedIndex_ > 0
			&& selectedIndex_ <= static_cast<int>(entries_.size())) {
			previewName = entries_[selectedIndex_ - 1].name_.c_str();
		}

		if (ImGui::BeginCombo("編集対象", previewName)) {
			if (ImGui::Selectable("None", selectedIndex_ == 0)) {
				selectedIndex_ = 0;
			}

			for (int index = 0;
				index < static_cast<int>(entries_.size());
				++index) {
				const bool isSelected = selectedIndex_ == index + 1;

				if (ImGui::Selectable(
					entries_[index].name_.c_str(),
					isSelected
				)) {
					selectedIndex_ = index + 1;
				}
			}

			ImGui::EndCombo();
		}

		// ──────────────── 選択対象だけを保存・復元 ───────────────
		if (spriteSave_
			&& selectedIndex_ > 0
			&& selectedIndex_ <= static_cast<int>(entries_.size())) {
			const std::string& selectedName =
				entries_[selectedIndex_ - 1].name_;

			if (ImGui::Button("選択対象を保存")) {
				spriteSave_->SaveSettings(selectedName);
			}

			ImGui::SameLine();

			if (ImGui::Button("選択対象を元に戻す")) {
				spriteSave_->LoadSettings(selectedName);
			}
		}

		ImGui::Separator();

		// ──────────────── 選択対象の編集項目 ───────────────
		if (selectedIndex_ == 0) {
			ImGui::Text("編集するスプライトを選択してください");
		} else if (selectedIndex_ <= static_cast<int>(entries_.size())) {
			Entry& entry = entries_[selectedIndex_ - 1];
			ImGui::Text("[%s]", entry.name_.c_str());
			entry.callback_();
		}

		ImGui::End();

		// ──────────────── 画面上のスプライト選択 ───────────────
		// ImGuiウィンドウの操作中でなければ、画像クリックで選択する。
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			&& !ImGui::GetIO().WantCaptureMouse) {
			for (int index = static_cast<int>(entries_.size()) - 1;
				index >= 0;
				--index) {
				if (spriteGizmo_.IsMouseOver(entries_[index].sprite_)) {
					selectedIndex_ = index + 1;
					break;
				}
			}
		}

		// ──────────────── 選択中スプライトのマウス操作 ───────────────
		if (selectedIndex_ > 0
			&& selectedIndex_ <= static_cast<int>(entries_.size())) {
			spriteGizmo_.Update(
				entries_[selectedIndex_ - 1].sprite_
			);
		} else {
			spriteGizmo_.Clear();
		}
#endif
	}

} // namespace TKM
