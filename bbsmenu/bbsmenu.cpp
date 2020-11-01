/**
 *
 * @file    bbsmenu.cpp
 *
 * @brief   掲示板メニュー管理クラス実装
 *          板一覧データ取得＆パース処理
 *
 * @author  Taro.Yamada
 *
 * @date    2019/01/30
 */

#include <QStandardItemModel>
#include <QDebug>
#include <QFile>
#include <QNetworkAccessManager>
#include <QFileInfo>
#include <QDir>

#include "bbsmenu.h"
#include "settings.h"
#include "xmloutput.h"
#include "bbsmenucacheloader.h"
#include "bbsmenuwebloader.h"
#include "bbsmenusaver.h"
#include "qchexception.h"
#include "common.h"

namespace bbsmenu {

/**
 * @brief       BbsManu::BbsManu
 * @details     コンストラクタ
 *              無視カテゴリと無視スレッドをメンバ変数に読み込む
 *              ネットアクセス用のQNerwotkAccessManagerポインタを退避する
 * @param[in]   parent  親オブジェクトポインタ
 * @return      なし
 */
BbsMenu::BbsMenu(QNetworkAccessManager * const pManager, QObject * const parent) : QObject{parent}
  , _pcNetworkAccessManager{pManager}
{
    Q_ASSERT(pManager);
}

/**
 * @brief               BbsMenu::menuModel
 *                      メニュー用モデル生成
 * @param[in]   from    メニュー読込先指定(FROM_CACHE:キャッシュ FROM_WEB:ウェブ)
 * @return              メニュー用モデルデータポインタ
 * @note                読み込み開始時と終了時にシグナルをエミットする
 */
QStandardItemModel *BbsMenu::menuModel(const LOAD_FROM from) {
    emit beginLoadBbsMenu();    // 読込開始シグナル

    QStandardItemModel *pModel = nullptr;

    // すでに一回HTMLからDLしているメニューモデルを返す
    if (from == LOAD_FROM::FROM_CACHE) {
        BbsMenuCacheLoader b;
        pModel = b.loadBbsMenu();
        if (pModel)
            LOGGING("BBS Menu load from cache.");
    }

    if (!pModel) {  // キャッシュから読み込めなかった場合にもWeb経由で取得する
        // ネットからメニューのHTMLを読み込みパースする
        BbsMenuWebLoader b{_pcNetworkAccessManager};
        pModel = b.loadBbsMenu();
        if (pModel)
            LOGGING("BBS Menu load from web.");
    }

    emit endLoadBbsMenu();  // 読込終了シグナル

    if (!pModel)
        QCH_EXCEPTION("BBS Menu load fail. from: " + static_cast<int>(from));

    return pModel;
}

/**
 * @brief       BbsManu::saveBbsMenu
 *              カテゴリリスト保存
 * @param[in]   pModel  保存するカテゴリモデル
 * @return      処理結果
 * @retval      true    正常終了
 * @retval      false   エラー
 */
void BbsMenu::saveBbsMenu(const QStandardItemModel * const pModel) {
    Q_ASSERT(pModel);

    BbsMenuSaver s{pModel};
    s.save();
}

}   // end of namespace
