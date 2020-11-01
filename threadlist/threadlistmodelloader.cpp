#include <QStandardItemModel>
#include <QRegularExpression>

#include "threadlistmodelloader.h"
#include "qchexception.h"
#include "common.h"
#include "threadlistloader.h"
#include "TLDefine.h"

namespace threadlist {

/**
 * @brief       ThreadListModelLoader::ThreadListModelLoader
 *              コンストラクタ
 * @param[in]   parent  親オブジェクト
 */
ThreadListModelLoader::ThreadListModelLoader(QObject *parent) : QObject(parent)
{
}

/**
 * @brief       ThreadListModelLoader::model
 *              スレッド一覧モデルオブジェクト取得処理(Web)
 * @param[in]   strUrl      スレッド一覧HTMLURL
 * @param[in]   pManager    ネットワークアクセスオブジェクト
 * @return      取得したモデル(エラー時は空のモデル)
 */
QStandardItemModel *ThreadListModelLoader::model(const QString strUrl, QNetworkAccessManager *pManager)
{
    const static QString STR_THREAD_LINE_HEADLINE_RE =
        R"((\d{4}/\d{2}/\d{2}\s\d{2}:\d{2}:\d{2})\s<a\shref.*?=(.*?)>(.*?)</a>)";
    const static QString STR_THREAD_LINE_NO_HEADLINE_RE = R"(<a\shref.*?="(.*?)/l50">(\d+):\s(.*?)</a>)";

    auto pModel = new QStandardItemModel;

    // スレッド一覧のHTMLを読み込む
    QString strThreadsHtml;
    bool isHeadline;
    if (!loadThreadHtml(&strThreadsHtml, &isHeadline, pManager, strUrl)) {
        LOGGING("Get Thread List failed.");
        return pModel;    // 読み込み失敗時は空のQStandardItemModelを返す
    }

    // スレッド行抽出用正規表現を設定
    QString STR_THREAD_LINE_RE = isHeadline ? STR_THREAD_LINE_HEADLINE_RE : STR_THREAD_LINE_NO_HEADLINE_RE;

    // スレッド定義のHTML行がある限り繰り返し処理を行う
    QRegularExpression reThreadLine {STR_THREAD_LINE_RE, QRegularExpression::CaseInsensitiveOption};
    int nThreadNumber = 1;
    int nIndex = 0;
    while (true) {
        // 一致行取得
        QRegularExpressionMatch remThread = reThreadLine.match(strThreadsHtml, nIndex);
        if (!remThread.hasMatch())
            break;

        // 日付、スレッドURL、スレッドタイトルの取得
        QDateTime cDT;
        QString strThreadUrl;
        QString strTitle;
        if (isHeadline) // ヘッドライン
            extractHeadlineData(remThread, &cDT, &strThreadUrl, &strTitle);
        else            // 非ヘッドライン
            extractNonHeadlineData(remThread, &strThreadUrl, &strTitle, &nThreadNumber, strUrl);

        // スレッドタイトル整形
        strTitle = arrangeTitle(strTitle, isHeadline);

        // アイテム生成&追加
        addThreadItem(pModel, nThreadNumber, strTitle, cDT, strThreadUrl);

        nIndex = remThread.capturedEnd() + 1;   // 次回マッチ検索開始位置更新
    }

    return pModel;
}

/**
 * @brief       ThreadListModelLoader::model
 *              スレッド一覧モデルオブジェクト取得処理(Web)
 * @param[in]   strFilename スレッド一覧ファイルパス
 * @return      取得したモデル(エラー時は空のモデル)
 */
QStandardItemModel *ThreadListModelLoader::model(const QString strFilename)
{
    /// @todo 実装
    return new QStandardItemModel(this);
}

/**
 * @brief       ThreadListModelLoader::loadThreadHtml
 *              スレッド一覧のHTML読込
 * @param[out]  html        取得したHTML
 * @param[out]  isHeadline  ヘッドラインかどうか
 * @return      処理結果
 */
bool ThreadListModelLoader::loadThreadHtml(QString *html, bool *isHeadline, QNetworkAccessManager *pManager
                                           , const QString strUrl)
{
    ThreadListLoader tll {pManager};
    std::pair<QString, bool> loadResult = tll.load(strUrl);
    *html = loadResult.first;
    *isHeadline = loadResult.second;    // ヘッドラインの場合はXML構造が異なる
    if (html->isEmpty() || html->isNull())
        return false;

    return true;
}

/**
 * @brief       ThreadListModelLoader::extractHeadlineData
 *              ヘッドライン情報を取得する
 * @param[in]   remThread       正規表現マッチオブジェクト
 * @param[out]  cDT             取得した日付
 * @param[out]  strThreadUrl    スレッドHTMLのURL
 * @param[out]  strTitle        スレッドタイトル
 */
void ThreadListModelLoader::extractHeadlineData(const QRegularExpressionMatch &remThread, QDateTime *cDT
                                                , QString *strThreadUrl, QString *strTitle) const
{
    QString strDateTime = remThread.captured(1);
    *cDT = QDateTime::fromString(strDateTime, "yyyy/MM/dd hh:mm:ss");
    *strThreadUrl = remThread.captured(2);
    *strTitle = remThread.captured(3).trimmed();
}

/**
 * @brief       ThreadListModelLoader::extractNonHeadlineData
 *              非ヘッドライン情報を取得する
 * @param[in]   remThread       正規表現マッチオブジェクト
 * @param[out]  strThreadUrl    スレッドURL
 * @param[out]  strTitle        スレッドタイトル
 * @param[out]  nThreadNumber   スレッド番号
 */
void ThreadListModelLoader::extractNonHeadlineData(const QRegularExpressionMatch &remThread, QString *strThreadUrl
                                                   , QString *strTitle, int *nThreadNumber, const QString strUrl) const
{
    *strThreadUrl = strUrl;
    int nCgiInsertPos = strThreadUrl->lastIndexOf('/', strThreadUrl->length() - 2);
    strThreadUrl->insert(nCgiInsertPos, "/test/read.cgi");
    *strThreadUrl = *strThreadUrl + remThread.captured(1);
    *strTitle = remThread.captured(3).trimmed();
    *nThreadNumber = remThread.captured(2).toInt();
}

/**
 * @brief           ThreadListModelLoader::arrangeTitle
 *                  タイトルHTML文字列からタイトル部分を取得する
 * @param[in,out]   strTitleHtml    タイトルHTML文字列
 * @param[in]       isHeadline      ヘッドラインかどうか
 */
QString ThreadListModelLoader::arrangeTitle(const QString strTitleHtml, const bool isHeadline)
{
    QString strTitle = strTitleHtml;
    strTitle.remove(QRegularExpression(R"(<font\scolor.*?=.*?>)", QRegularExpression::CaseInsensitiveOption));
    strTitle.remove(QRegularExpression("</font>", QRegularExpression::CaseInsensitiveOption));
    strTitle.replace("&#169;", "(c)");
    if (!isHeadline)
        strTitle.remove(QRegularExpression(R"(\s\(\d+\))"));

    return strTitle;
}

/**
 * @brief       ThreadListModelLoader::addThreadItem
 *              モデルへのアイテム追加
 * @param[in]   pModel          追加するモデル
 * @param[in]   nThreadNumber   スレッド番号
 * @param[in]   strTitle        スレッドタイトル
 * @param[in]   cDT             日付
 * @param[in]   strThreadUrl    スレッドURL
 */
void ThreadListModelLoader::addThreadItem(QStandardItemModel *pModel, const int nThreadNumber, const QString strTitle
                               , const QDateTime cDT, const QString strThreadUrl) const
{
    auto pItem = new QStandardItem{strTitle};
    THREAD_TITLE_DATA sData { nThreadNumber, cDT, strThreadUrl, strTitle, 0 };
    pItem->setData(QVariant::fromValue(sData));
    pModel->appendRow(pItem);
}

}   // end of namespace
