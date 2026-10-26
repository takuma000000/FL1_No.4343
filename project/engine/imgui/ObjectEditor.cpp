#include "ObjectEditor.h"

#include <algorithm>
#include <utility>

#ifdef USE_IMGUI
#include "imgui.h"
#endif

namespace TKM {

	void ObjectEditor::SetObjectSave(ObjectSave* objectSave) {
		objectSave_ = objectSave;
	}

	bool ObjectEditor::RegisterObject(
		const std::string& name,
		ImGuiCallback callback
	) {
		if (name.empty() || !callback) {
			return false;
		}

		// 同じ名前がすでに登録されていないか確認する
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

		entries_.push_back(
			Entry{
				name,
				std::move(callback)
			}
		);

		return true;
	}

	void ObjectEditor::UnregisterObject(const std::string& name) {
		std::erase_if(
			entries_,
			[&name](const Entry& entry) {
				return entry.name_ == name;
			}
		);

		// 登録解除後は選択をNoneへ戻す
		selectedIndex_ = 0;
	}

	void ObjectEditor::Clear() {
		entries_.clear();
		selectedIndex_ = 0;
	}

	void ObjectEditor::ImGuiDebug() {
#ifdef USE_IMGUI
		ImGui::Begin("オブジェクト編集");

		// ──────────────── オブジェクト保存 ───────────────
		if (objectSave_) {

			if (ImGui::Button("全オブジェクトを保存")) {
				objectSave_->SaveSettings();
			}

			ImGui::SameLine();

			if (ImGui::Button("最後の保存状態に戻す")) {
				objectSave_->LoadSettings();
			}

			const std::string& message =
				objectSave_->GetSettingsMessage();

			if (!message.empty()) {
				ImGui::TextWrapped("%s", message.c_str());
			}
		}

		// ──────────────── オブジェクト選択 ───────────────
		ImGui::Separator();
		const char* previewName = "None";

		if (selectedIndex_ > 0
			&& selectedIndex_ <= static_cast<int>(entries_.size())) {
			previewName = entries_[selectedIndex_ - 1].name_.c_str();
		}

		if (ImGui::BeginCombo("編集対象", previewName)) {
			const bool isNoneSelected = selectedIndex_ == 0;

			if (ImGui::Selectable("None", isNoneSelected)) {
				selectedIndex_ = 0;
			}

			for (int index = 0;
				index < static_cast<int>(entries_.size());
				++index) {
				const bool isSelected =
					selectedIndex_ == index + 1;

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
			ImGui::Text(
				"編集するオブジェクトを選択してください"
			);
		} else if (
			selectedIndex_ <= static_cast<int>(entries_.size())
			) {
			Entry& entry = entries_[selectedIndex_ - 1];

			ImGui::Text("[%s]", entry.name_.c_str());
			entry.callback_();
		}

		ImGui::End();
#endif
	}

} // namespace TKM