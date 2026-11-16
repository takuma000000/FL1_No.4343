#include "EnemyDataLoader.h"

#include <exception>
#include <fstream>
#include <string>
#include <cmath>

#include "../EnemyAttribute.h"
#include "../EnemyType.h"
#include "json.hpp"

bool EnemyDataLoader::LoadFromFile(
	const std::filesystem::path& filePath,
	EnemyData& outputData,
	std::string& errorMessage
) {
	// 前回のエラー内容が残らないように初期化する。
	errorMessage.clear();

	// 日本語を含むパスにも対応できるよう、
	// std::filesystem::pathを直接ifstreamへ渡す。
	std::ifstream file(filePath);

	if (!file.is_open()) {
		errorMessage =
			"敵データファイルを開けませんでした: " +
			filePath.string();

		return false;
	}

	try {
		nlohmann::json root;
		file >> root;

		// 読み込み途中で失敗してもoutputDataを変更しないよう、
		// 一時変数へすべての情報を格納する。
		EnemyData loadedData{};

		//=========================================================
		// 基本情報
		//=========================================================
		loadedData.id =
			root.at("id").get<std::string>();

		loadedData.displayName =
			root.at("displayName").get<std::string>();

		loadedData.modelName =
			root.at("modelName").get<std::string>();

		//=========================================================
		// EnemyType
		//=========================================================
		const std::string typeText =
			root.at("type").get<std::string>();

		const std::optional<EnemyType> enemyType =
			EnemyTypeFromString(typeText);

		if (!enemyType.has_value()) {
			errorMessage =
				"未定義のEnemyTypeです: " +
				typeText;

			return false;
		}

		loadedData.type = enemyType.value();

		//=========================================================
		// ステータス
		//=========================================================
		const nlohmann::json& status =
			root.at("status");

		loadedData.maxHp =
			status.at("maxHp").get<int>();

		loadedData.moveSpeed =
			status.at("moveSpeed").get<float>();

		loadedData.attackPower =
			status.at("attackPower").get<float>();

		//=========================================================
		// 戦闘情報
		//=========================================================
		const nlohmann::json& combat =
			root.at("combat");

		loadedData.detectionRange =
			combat.at("detectionRange").get<float>();

		loadedData.attackRange =
			combat.at("attackRange").get<float>();

		loadedData.attackInterval =
			combat.at("attackInterval").get<float>();

		loadedData.collisionRadius =
			combat.at("collisionRadius").get<float>();

		//=========================================================
		// 属性情報
		//=========================================================
		const nlohmann::json& attribute =
			root.at("attribute");

		const std::string attributeText =
			attribute.at("type").get<std::string>();

		const std::string weaknessText =
			attribute.at("weakness").get<std::string>();

		const std::string resistanceText =
			attribute.at("resistance").get<std::string>();

		const std::optional<EnemyAttribute> enemyAttribute =
			EnemyAttributeFromString(attributeText);

		const std::optional<EnemyAttribute> weakness =
			EnemyAttributeFromString(weaknessText);

		const std::optional<EnemyAttribute> resistance =
			EnemyAttributeFromString(resistanceText);

		if (!enemyAttribute.has_value()) {
			errorMessage =
				"未定義の敵属性です: " +
				attributeText;

			return false;
		}

		if (!weakness.has_value()) {
			errorMessage =
				"未定義の弱点属性です: " +
				weaknessText;

			return false;
		}

		if (!resistance.has_value()) {
			errorMessage =
				"未定義の耐性属性です: " +
				resistanceText;

			return false;
		}

		loadedData.attribute =
			enemyAttribute.value();

		loadedData.weakness =
			weakness.value();

		loadedData.resistance =
			resistance.value();

		//=========================================================
		// 撃破報酬
		//=========================================================
		const nlohmann::json& reward =
			root.at("reward");

		loadedData.antibodyPoint =
			reward.at("antibodyPoint").get<int>();

		//=========================================================
		// Transform
		//=========================================================
		const nlohmann::json& transform =
			root.at("transform");

		const nlohmann::json& scale =
			transform.at("scale");

		if (!scale.is_array() || scale.size() != 3) {
			errorMessage =
				"transform.scaleは3要素の配列である必要があります。";

			return false;
		}

		loadedData.scale = {
			scale.at(0).get<float>(),
			scale.at(1).get<float>(),
			scale.at(2).get<float>()
		};

        //=========================================================
        // 読み込み値の検証
        //=========================================================
		if (!Validate(loadedData, errorMessage)) {
			return false;
		}      

		// すべての読み込みに成功した場合のみ、
		// 呼び出し側のEnemyDataへ結果を反映する。
		outputData = loadedData;

		return true;
	}
	catch (const nlohmann::json::exception& exception) {
		errorMessage =
			"敵データJSONの解析に失敗しました: " +
			std::string(exception.what());

		return false;
	}
	catch (const std::exception& exception) {
		errorMessage =
			"敵データの読み込み中にエラーが発生しました: " +
			std::string(exception.what());

		return false;
	}
}

bool EnemyDataLoader::Validate(
	const EnemyData& data,
	std::string& errorMessage
) {
	//=========================================================
	// 基本文字列
	//=========================================================
	if (data.id.empty()) {
		errorMessage =
			"敵データのidが空です。";

		return false;
	}

	if (data.displayName.empty()) {
		errorMessage =
			"敵データのdisplayNameが空です: " +
			data.id;

		return false;
	}

	if (data.modelName.empty()) {
		errorMessage =
			"敵データのmodelNameが空です: " +
			data.id;

		return false;
	}

	//=========================================================
	// ステータス
	//=========================================================
	if (data.maxHp <= 0) {
		errorMessage =
			"maxHpは1以上である必要があります: " +
			data.id;

		return false;
	}

	if (
		!std::isfinite(data.moveSpeed) ||
		data.moveSpeed < 0.0f
		) {
		errorMessage =
			"moveSpeedは0以上の有限値である必要があります: " +
			data.id;

		return false;
	}

	if (
		!std::isfinite(data.attackPower) ||
		data.attackPower < 0.0f
		) {
		errorMessage =
			"attackPowerは0以上の有限値である必要があります: " +
			data.id;

		return false;
	}

	//=========================================================
	// 戦闘パラメータ
	//=========================================================
	if (
		!std::isfinite(data.detectionRange) ||
		data.detectionRange < 0.0f
		) {
		errorMessage =
			"detectionRangeは0以上の有限値である必要があります: " +
			data.id;

		return false;
	}

	if (
		!std::isfinite(data.attackRange) ||
		data.attackRange < 0.0f
		) {
		errorMessage =
			"attackRangeは0以上の有限値である必要があります: " +
			data.id;

		return false;
	}

	if (data.detectionRange < data.attackRange) {
		errorMessage =
			"detectionRangeはattackRange以上である必要があります: " +
			data.id;

		return false;
	}

	if (
		!std::isfinite(data.attackInterval) ||
		data.attackInterval <= 0.0f
		) {
		errorMessage =
			"attackIntervalは0より大きい有限値である必要があります: " +
			data.id;

		return false;
	}

	if (
		!std::isfinite(data.collisionRadius) ||
		data.collisionRadius <= 0.0f
		) {
		errorMessage =
			"collisionRadiusは0より大きい有限値である必要があります: " +
			data.id;

		return false;
	}

	//=========================================================
	// Transform
	//=========================================================
	const bool isScaleValid =
		std::isfinite(data.scale.x) &&
		std::isfinite(data.scale.y) &&
		std::isfinite(data.scale.z) &&
		data.scale.x > 0.0f &&
		data.scale.y > 0.0f &&
		data.scale.z > 0.0f;

	if (!isScaleValid) {
		errorMessage =
			"scaleの各要素は0より大きい有限値である必要があります: " +
			data.id;

		return false;
	}

	//=========================================================
	// 報酬
	//=========================================================
	if (data.antibodyPoint < 0) {
		errorMessage =
			"antibodyPointは0以上である必要があります: " +
			data.id;

		return false;
	}

	return true;
}