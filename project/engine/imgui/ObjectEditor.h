#pragma once
#include "ObjectSave.h"

#include <functional>
#include <string>
#include <vector>

namespace TKM {

	//=============================================================
	// ObjectEditorクラス
	// 登録されたオブジェクトの選択とImGui編集を管理するクラス。
	// オブジェクト本体の所有権は持たない。
	//=============================================================
	class ObjectEditor {
	public:
		// オブジェクト固有のImGuiを表示する処理
		using ImGuiCallback = std::function<void()>;

		/// <summary>
		/// オブジェクト保存機能を設定します。
		/// </summary>
		/// <param name="objectSave">使用する保存機能</param>
		void SetObjectSave(ObjectSave* objectSave);

		/// <summary>
		/// 編集対象を登録します。
		/// </summary>
		/// <param name="name">選択欄へ表示する名前</param>
		/// <param name="callback">ImGuiを表示する処理</param>
		/// <returns>登録できた場合はtrue</returns>
		bool RegisterObject(
			const std::string& name,
			ImGuiCallback callback
		);

		/// <summary>
		/// 指定した編集対象の登録を解除します。
		/// </summary>
		/// <param name="name">登録時に指定した名前</param>
		void UnregisterObject(const std::string& name);

		/// <summary>
		/// 編集対象をすべて解除します。
		/// </summary>
		void Clear();

		/// <summary>
		/// オブジェクト編集画面を表示します。
		/// </summary>
		void ImGuiDebug();

	private:
		//=============================================================
		// 登録情報
		//=============================================================

		struct Entry {
			std::string name_;
			ImGuiCallback callback_;
		};

		std::vector<Entry> entries_;

		// 0はNone、1以降はentries_の添字に1を足した値
		int selectedIndex_ = 0;

		//=============================================================
		// オブジェクト保存
		//=============================================================

		ObjectSave* objectSave_ = nullptr;
	};

} // namespace TKM