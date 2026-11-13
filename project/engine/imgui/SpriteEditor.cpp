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
		ImGuiCallback callback
	) {
		if (name.empty() || !callback) {
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

		entries_.push_back({ name, std::move(callback) });
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
#endif
	}

} // namespace TKM
