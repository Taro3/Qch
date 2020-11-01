/**
 * @file    bbsmenucacheloader.cpp
 *
 * @brief   板一覧(メニュー一覧)キャッシュデータ読み込みクラス実装
 *
 * @author  Taro.Yamada
 *
 * @date    2019/09/13
 */

#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>
#include <QStandardItemModel>

#include "bbsmenucacheloader.h"
#include "settings.h"
#include "filepath.h"
#include "qchexception.h"
#include "common.h"

namespace bbsmenu {

/**
 * @brief       BbsMenuCacheLoader::BbsMenuCacheLoader
 *              コンストラクタ
 * @param[in]   parent  親オブジェクトポインタ
 */
BbsMenuCacheLoader::BbsMenuCacheLoader(QObject *parent) : BbsMenuLoaderBase{parent} {
}

/**
 * @brief       BbsMenuCacheLoader::loadBbsMenu
 *              BBSメニュー読込処理(キャッシュ)
 * @return      読み込まれたメニューのアイテムモデルポインタ
 */
QStandardItemModel *BbsMenuCacheLoader::loadBbsMenu() {
    QString strFilename = FilePath::bbsmenuFilePathName();
    QFile cFile{strFilename};

    // XMLファイルオープン
    if (!cFile.open(QIODevice::ReadOnly | QIODevice::Text))
        QCH_EXCEPTION(strFilename + " can't open.");

    // XMLファイル読込&先頭読み込みチェック
    QXmlStreamReader xml{&cFile};
    if (xml.readNext() != QXmlStreamReader::StartDocument)
        QCH_EXCEPTION("invalid bbsmenu data format.");

    QStandardItemModel *pModel = readCategories(xml);

    return pModel;
}

/**
 * @brief BbsMenuCacheLoader::readCategories
 *                  カテゴリリストのXMLからの読込
 * @param[in,out]   xml カテゴリリストを保持するQXmlStreamReaderオブジェクト参照
 * @return          1カテゴリの名称とスレットリストを保持するQStandardItemModelオブジェクトポインタ
 */
QStandardItemModel *BbsMenuCacheLoader::readCategories(QXmlStreamReader &xml) const noexcept {
    xml.readNextStartElement();
    int nCategoryCount = xml.attributes().value("count").toInt();

    auto pModel = new QStandardItemModel;

    for (auto i = 0; i < nCategoryCount; ++i) {
        xml.readNextStartElement();

        QStandardItem *pItem = createThreadList(xml);
        pModel->appendRow(pItem);
    }

    LOGGING("Category count: " + QString::number(nCategoryCount));

    return pModel;
}

/**
 * @brief           BbsMenuCacheLoader::createThreadList
 *                  XMLから1カテゴリ内のスレッドリストを生成する
 * @param[in,out]   xml カテゴリリストを保持するQXmlStreamReaderオブジェクト参照
 * @return          指定カテゴリ内のスレッドのタイトルとURLを保持するQStandardItemオブジェクトポインタ
 */
QStandardItem *BbsMenuCacheLoader::createThreadList(QXmlStreamReader &xml) const {
    QString strCategoryName = xml.attributes().value("name").toString();   // カテゴリ名
    LOGGING("Category name: " + strCategoryName);

    auto pItem = new QStandardItem{strCategoryName};

    readThreadData(xml, pItem);

    xml.readNextStartElement(); // </category>タグを読み飛ばす

    return pItem;
}

/**
 * @brief           BbsMenuCacheLoader::readThreadData
 *                  カテゴリ内に含まれるスレッドタイトルとURLをアイテムの子として登録する
 * @param[in,out]   xml     スレッドタイトルとURLを保持するQXMLStreamReaderオブジェクト
 * @param[in]       pItem   スレッドタイトルとURLを登録するQStandardItemオブジェクトポインタ
 * @return          なし
 */
void BbsMenuCacheLoader::readThreadData(QXmlStreamReader &xml, QStandardItem * const pItem) const {
    int nItemCount = xml.attributes().value("count").toInt();          // スレッド数
    LOGGING("Thread count: " + QString::number(nItemCount));

    for (auto i = 0; i < nItemCount; ++i) {
        QPair<QString, QString> titleAndUrl = readTitleAndUrl(xml);
        LOGGING("Thread title: " + titleAndUrl.first + " Thread URL: " + titleAndUrl.second);

        auto pThreadItem = new QStandardItem{titleAndUrl.first};        // スレッドタイトル
        pThreadItem->setData(titleAndUrl.second);                       // URL
        pItem->appendRow(pThreadItem);
    }
}

/**
 * @brief           BbsMenuCacheLoader::readTitleAndUrl
 *                  XMLからスレッドタイトルとスレッドURLを取得する
 * @param[in,out]   xml データを読み取るQXmlStreamReaderオブジェクト
 * @return          取得したスレッドタイトルとスレッドURL文字列のQPairオブジェクト
 */
QPair<QString, QString> BbsMenuCacheLoader::readTitleAndUrl(QXmlStreamReader &xml) const {
    xml.readNextStartElement();
    QString strTitle = xml.readElementText();                          // スレッドタイトル

    xml.readNextStartElement();
    QString strUrl = xml.readElementText();                            // スレッドURL

    return QPair<QString, QString>{strTitle, strUrl};
}

}   // end of namespace
