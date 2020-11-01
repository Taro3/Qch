/**
 * @file    bbsmenuwebloader.cpp
 *
 * @brief   板一覧(メニュー一覧)ウェブデータ読み込みクラス実装
 *
 * @author  Taro.Yamada
 *
 * @date    2019/09/17
 */

#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextCodec>
#include <QRegularExpression>
#include <QStandardItemModel>

#include "bbsmenuwebloader.h"
#include "settings.h"
#include "qchexception.h"
#include "common.h"

namespace bbsmenu {

/**
 * @brief       BbsMenuWebLoader::BbsMenuWebLoader
 *              コンストラクタ
 * @param[in]   pManager    ネットアクセスオブジェクト
 * @param[in]   parent      親オブジェクト
 */
BbsMenuWebLoader::BbsMenuWebLoader(QNetworkAccessManager * const pManager, QObject *parent) : BbsMenuLoaderBase{parent}
  , _pManager{pManager}
{
}

/**
 * @brief   BbsMenuWebLoader::loadBbsMenu
 *          BBSメニューのモデル取得(Web)
 * @return  メニューモデル
 */
QStandardItemModel *BbsMenuWebLoader::loadBbsMenu() {
    QStringList menuHtml = loadBbsMenuFromWeb();
    return parseBbsMenu(menuHtml);
}

/**
 * @brief   BbsMenuWebLoader::loadBbsMenuFromWeb
 *          ホストからメニューのHTMLを取得する
 * @return  メニューHTMLデータ
 *          エラーの場合は空のQStringListを返す
 * @todo    アクセス終了をシグナル/スロット処理に変更する
 */
QStringList BbsMenuWebLoader::loadBbsMenuFromWeb() {
    QStringList clstMenu;
    Settings cSetting;

    QString strUrl = cSetting.value(Settings::eSETTING_NAME::SETTING_BBSMENU_URL);
    QNetworkReply *pcReply = _pManager->get(QNetworkRequest(QUrl(strUrl)));
    while (pcReply->isRunning())
        qApp->processEvents();

    if (pcReply->error() != QNetworkReply::NetworkError::NoError) {
        LOGGING(pcReply->errorString());
        pcReply->close();
        return clstMenu;
    }

    QByteArray baMenuHtml  = pcReply->readAll();
    QTextCodec *pHtmlCodec = QTextCodec::codecForHtml(baMenuHtml);
    QString strMenuHtml = pHtmlCodec->toUnicode(baMenuHtml);
    clstMenu = strMenuHtml.split('\n');

    return clstMenu;
}

/**
 * @brief       BbsMenuWebLoader::parseBbsMenu
 * 				メニューのHTMLを解析しデータに分割する
 * @param[in]	clstMenuHtml	メニューのHTML
 * @return      HTMLから生成されたアイテムモデル
 */
QStandardItemModel *BbsMenuWebLoader::parseBbsMenu(const QStringList clstMenuHtml) {
    static const QString CATEGORY_PATTERN = "^<br><br><B>.*?</B><br>.*";
    static const QRegularExpression reCategory{CATEGORY_PATTERN, QRegularExpression::CaseInsensitiveOption};

    auto pModel = new QStandardItemModel;
    int idx = 0;
    while ((idx = clstMenuHtml.indexOf(reCategory, idx)) >= 0) {
        QString strLine = clstMenuHtml[idx];

        // カテゴリ名文字列取得
        QString strCategoryName = categoryName(strLine);
        if (strCategoryName.isEmpty()) {
            // マッチカテゴリなしまたは無視カテゴリ
            ++idx;
            continue;
        }

        auto pCategoryItem = new QStandardItem{strCategoryName};
        // カテゴリアイテムの子としてスレッド名とURLを追加する
        idx = buildMenuModel(++idx, clstMenuHtml, pCategoryItem);
        pModel->appendRow(pCategoryItem);
    }

    return pModel;
}

/**
 * @brief           BbsMenuWebLoader::buildMenuModel
 *                  カテゴリアイテムの子アイテムとしてスレッド名＋URLのアイテムを追加する
 * @param[in]       startLineIndex    HTML内のスレッド文字列検索開始行インデックス
 * @param[in]       menuHtml          カテゴリ内のスレッド一覧を含むHTML
 * @param[in,out]   pCategoryItem     スレッド一覧を追加するカテゴリアイテム
 * @return          次カテゴリの検索を開始するHTMLの行インデックス番号
 */
int BbsMenuWebLoader::buildMenuModel(const int startLineIndex, const QStringList menuHtml
                                     , QStandardItem * const pCategoryItem)
{
    // スレッドリンク行処理
    int nHtmlThreadLineIndex = startLineIndex;
    forever {
        QString strThreadLine = menuHtml[nHtmlThreadLineIndex];
        // 行が空?
        if (strThreadLine.isEmpty()) {
            ++nHtmlThreadLineIndex;
            continue;
        }

        QString strThreadName;
        QString strThreadUrl;
        // マッチなし?
        if (!extractTitleAndUrlFromHtml(strThreadLine, &strThreadName, &strThreadUrl))
            break;

        // コメント行または無視スレッド
        if (strThreadName.isEmpty() && strThreadUrl.isEmpty()) {
            ++nHtmlThreadLineIndex;
            continue;
        }

        auto pThreadItem = new QStandardItem{strThreadName};    // 子アイテム生成
        pThreadItem->setData(strThreadUrl);                     // data(USER_ROLE + 1) に URL をセット
        pCategoryItem->appendRow(pThreadItem);                  // カテゴリの子として追加する

        ++nHtmlThreadLineIndex;
    }

    return nHtmlThreadLineIndex;
}

/**
 * @brief   BbsMenuWebLoader::extractTitleAndUrlFromHtml
 *          メニューのHTMLからスレッドタイトルとURLを抽出する
 * @param[in]   strLineText HTMLの行データ
 * @param[out]  title       スレッドタイトル文字列
 * @param[out]  url         スレッドURL文字列
 * @return  処理結果
 * @retval  true    一致あり
 * @retval  false   一致なし
 * @note    コメント行または無視スレッドの場合はtitleとurl共に空文字列を設定する
 */
bool BbsMenuWebLoader::extractTitleAndUrlFromHtml(const QString strLineText, QString *title, QString *url) {
    static const QString THREAD_LINE_PATTERN = "^<A\\sHREF.*?=.*?(http://.*?)>(.*?)</A>.*";
    static const QRegularExpression reThreadLine{THREAD_LINE_PATTERN, QRegularExpression::CaseInsensitiveOption};
    static const QRegularExpression reCommentLine{"^<!--.*?-->", QRegularExpression::CaseInsensitiveOption};

    title->clear();
    url->clear();

    // コメント?
    if (strLineText.indexOf(reCommentLine) >= 0)
        return true;

    // マッチなし?
    QRegularExpressionMatch reMatchThreadLine = reThreadLine.match(strLineText);
    if (!reMatchThreadLine.hasMatch())
        return false;

    QString tmpTitle = reMatchThreadLine.captured(2).trimmed();    // タイトル
    QString tmpUrl = reMatchThreadLine.captured(1).trimmed();      // URL

    // 無視スレッド名?
    Settings s;
    if (s.ignoreThreads().indexOf(tmpTitle) >= 0)
        return true;

    *title = tmpTitle;
    *url = tmpUrl;

    return true;
}

/**
 * @brief       BbsMenuWebLoader::categoryName
 *              メニューのHTML1行からカテゴリ名を抽出する
 * @param[in]   strLineText HTML行データ
 * @return      カテゴリ名
 *              無視カテゴリ名またはエラーの場合は空文字列を返す
 */
QString BbsMenuWebLoader::categoryName(const QString strLineText) {
    static const QRegularExpression reCategoryTitle{"<B>(.*?)</B>", QRegularExpression::CaseInsensitiveOption};

    QRegularExpressionMatch reMatchCategory = reCategoryTitle.match(strLineText);
    if (!reMatchCategory.hasMatch())
        return "";
    QString strCategoryName = reMatchCategory.captured(1).trimmed();

    // 無視カテゴリリストチェック
    Settings s;
    if (s.ignoreCategories().indexOf(strCategoryName) >= 0)
        return "";

    return strCategoryName;
}

}   // end of namespace
