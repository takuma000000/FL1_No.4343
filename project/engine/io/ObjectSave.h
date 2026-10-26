#pragma once
#include "MyMath.h"

#include <functional>
#include <map>
#include <optional>
#include <string>

namespace TKM {

	//=============================================================
	// ObjectSaveクラス
	// オブジェクトの保存対象を登録・管理するクラス。
	//=============================================================
	class ObjectSave {
	public:
		//=============================================================
		// 保存データ
		//=============================================================

		// オブジェクトから保存する値
		struct Values {
			Vector3 position_;
			Vector3 rotation_;
			Vector3 scale_;

			// 表示状態を持つオブジェクトだけ使用する
			std::optional<bool> visible_;
		};

		// オブジェクトから現在値を取得する処理
		using Getter = std::function<Values()>;

		// 保存した値をオブジェクトへ反映する処理
		using Setter = std::function<void(const Values&)>;

		/// <summary>
		/// 保存対象を登録します。
		/// </summary>
		/// <param name="id">オブジェクトを識別する名前</param>
		/// <param name="getter">現在値を取得する処理</param>
		/// <param name="setter">保存値を反映する処理</param>
		/// <returns>登録できた場合はtrue</returns>
		bool RegisterObject(
			const std::string& id,
			Getter getter,
			Setter setter
		);

		/// <summary>
		/// 指定した保存対象の登録を解除します。
		/// </summary>
		/// <param name="id">登録時に指定した名前</param>
		void UnregisterObject(const std::string& id);

		/// <summary>
		/// 保存対象をすべて解除します。
		/// </summary>
		void Clear();

		/// <summary>
		/// 登録されているオブジェクトを保存します。
		/// </summary>
		/// <returns>保存できた場合はtrue</returns>
		bool SaveSettings();
		/// <summary>
		/// 保存したオブジェクト設定を読み込みます。
		/// </summary>
		/// <returns>読み込めた場合はtrue</returns>
		bool LoadSettings();

		// Setter===================================
		/// <summary>
		/// 設定ファイルの保存先を指定します。
		/// </summary>
		/// <param name="filePath">設定ファイルのパス</param>
		void SetSettingsPath(const std::string& filePath) {
			settingsPath_ = filePath;
		}
		// =========================================
		// Getter===================================
		/// <summary>
		/// 保存・読込結果のメッセージを取得します。
		/// </summary>
		const std::string& GetSettingsMessage() const {
			return settingsMessage_;
		}
		// =========================================

	private:
		//=============================================================
		// 登録情報
		//=============================================================

		struct Entry {
			Getter getter_; // 現在値を取得する処理
			Setter setter_; // 保存値を反映する処理
		};

		// 登録情報のマップ（キーはオブジェクトの識別名）
		std::map<std::string, Entry> entries_;

		//======================================================================
		// 設定ファイル
		//======================================================================
		std::string settingsPath_;    // 設定ファイルの保存先
		std::string settingsMessage_; // 保存・読込結果
	};

} // namespace TKM