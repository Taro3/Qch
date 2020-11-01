/**
 * @file    filepath.cpp
 *
 * @brief   板一覧(メニュー一覧)ファイルパス取得クラス実装
 *
 * @author  Taro.Yamada
 *
 * @date    2019/09/17
 */

#include "filepath.h"
#include "settings.h"

namespace bbsmenu {

/**
 * @brief       FilePath::FilePath
 *              コンストラクタ
 * @param[in]   parent  親オブジェクト
 */
FilePath::FilePath(QObject *parent) : QObject{parent} {
}

/**
 * @brief FilePath::bbsmenuFilePathName
 *          bbsmenuファイルのパスとファイル名取得
 * @return  bbsmenuファイルのパスとファイル名
 */
QString FilePath::bbsmenuFilePathName() {
    Settings cSettings;
    QString strFilename = cSettings.value(Settings::eSETTING_NAME::SETTING_DATA_FILE_PATH) + "/"
            + cSettings.value(Settings::eSETTING_NAME::SETTING_BBSMENU_FILE_NAME);

    return strFilename;
}

}   // end of namespace
