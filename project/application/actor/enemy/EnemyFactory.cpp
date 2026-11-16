#include "EnemyFactory.h"

#include "data/EnemyDataCatalog.h"
#include "Object3dCommon.h"

std::unique_ptr<Enemy> EnemyFactory::Create(
    const std::string& enemyId,
    const EnemyDataCatalog& dataCatalog,
    const Vector3& position,
    TKM::DirectXCommon* dxCommon,
    TKM::Camera* camera,
    std::string& errorMessage
) {
    //======================================================================
    // エラー情報の初期化
    //======================================================================

    // 前回のエラー内容が残らないように初期化する
    errorMessage.clear();

    //======================================================================
    // 敵データの取得
    //======================================================================

    // JSONから読み込み済みのCatalogを敵IDで検索する。
    const EnemyData* enemyData = dataCatalog.FindById(enemyId);


    // 未登録のIDではEnemyを生成できないため、
    // エラー内容を設定してnullptrを返す。
    if (!enemyData) {
        errorMessage = "指定された敵IDがCatalogに登録されていません：" + enemyId;

        return nullptr;
    }

    //=========================================================
    // Enemyの生成
    //=========================================================
    auto enemy = std::make_unique<Enemy>();

    // Catalogから取得したEnemyDataを使用して、
    // 種類・モデル・HP・速度などをEnemyへ設定する。
    enemy->Initialize(
        enemyData->type,
        *enemyData,
        position,
        TKM::Object3dCommon::GetInstance(),
        dxCommon,
        camera
    );

    // 所有権を呼び出し側へ渡す。
    return enemy;
}