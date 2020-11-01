/**
 * @file    bbsmenusaver.cpp
 *
 * @brief   板一覧(メニュー一覧)キャッシュデータ保存処理実装
 *
 * @author  Taro.Yamada
 *
 * @date    2019/09/17
 */

#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

#include "bbsmenusaver.h"
#include "filepath.h"
#include "xmloutput.h"
#include "qchexception.h"
#include "common.h"

namespace bbsmenu {

/**
 * @brief       BbsMenuSaver::BbsMenuSaver
 *              コンストラクタ
 * @param[in]   pModel  アイテムモデル
 * @param[in]   parent  親オブジェクト
 */
BbsMenuSaver::BbsMenuSaver(const QStandardItemModel * const pModel, QObject *parent) : QObject{parent}, _pModel{pModel}
{
    Q_ASSERT(pModel);
}

/**
 * @brief   BbsMenuSaver::save
 *          BBSメニューデータ保存
 * @return  処理結果
 */
bool BbsMenuSaver::save() {
    // キャッシュファイルパス取得＆生成
    QString strFilename = FilePath::bbsmenuFilePathName();
    if (!makeCategoryFileDir(strFilename)) {
        LOGGING("cant make file path: " + strFilename);
        return false;
    }

    QFile cFile(strFilename);
    if (!cFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOGGING(strFilename + " cant create.");
        return false;
    }

    // XML出力
    XmlOutput xo;
    xo.outputCategoryList(&cFile, _pModel);

    cFile.flush();
    cFile.close();

    return true;
}

/**
 * @brief       BbsMenuSaver::makeCategoryFileDir
 *              カテゴリリストキャッシュファイルのパス生成(ファイル名まで含む)
 * @param[in]   strFilename パスを含めたカテゴリリストキャッシュファイル名
 * @return      処理結果
 */
bool BbsMenuSaver::makeCategoryFileDir(const QString strFilename) const {
    if (!QFile::exists(strFilename)) {
        QFileInfo cFileInfo{strFilename};
        QDir cDir = cFileInfo.absoluteDir();
        if (!cDir.exists()) {
            if (!cDir.mkpath(cDir.path())) {
                LOGGING("Can't make directory: " + cDir.path());
                return false;
            }
        }
    }

    return true;
}

}   // end of namespace
