#include <string>

#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextCodec>
#include <QRegularExpression>

#include "threadlistloader.h"
#include "hbbsutil.h"

namespace threadlist {

/**
 * @brief       ThreadListLoader::ThreadListLoader
 *              コンストラクタ
 * @param[in]   pManager    ネットワークアクセス用オブジェクト
 * @param[in]   parent      親オブジェクトポインタ
 * @return      なし
 */
ThreadListLoader::ThreadListLoader(QNetworkAccessManager *pManager, QObject *parent) : QObject {parent}
  , _pManager {pManager}
{
    Q_ASSERT(pManager);
}

/**
 * @brief   ThreadListLoader::load
 *          スレッド一覧HTMLデータ取得処理
 * @param[in]   strThreadListHtmlUrl    スレッド一覧URL
 * @return  取得データ
 * @retval  std::pair<取得したHTMLデータ, ヘッドラインフラグ>
 * @note    取得に失敗した場合は、std::pair<空文字列, false>を返す
 */
std::pair<QString, bool> ThreadListLoader::load(const QString strThreadListHtmlUrl) {
    bool isHeadline = true;
    HBBSUtil util;
    // 指定URLのスレッド一覧HTMLを取得する
    QString strThreadListHtml = util.receiveHtml(_pManager, strThreadListHtmlUrl);
    if (strThreadListHtml.isNull() || strThreadListHtml.isEmpty())
        return std::make_pair(QString(), false);

    // 非ヘッドラインの場合はURLを全件のページに変更してHTMLを取得する
    if (!isHeadlineHtml(strThreadListHtml)) {
        strThreadListHtml = loadNonHeadline(strThreadListHtmlUrl, util);
        if (strThreadListHtml.isEmpty() || strThreadListHtml.isNull())
            return std::make_pair(QString(), false);
        isHeadline = false;
    }
    // ヘッドラインの場合は取得済みのHTMLをそのまま使用する

    return std::make_pair(strThreadListHtml, isHeadline);
}

/**
 * @brief       ThreadListLoader::isHeadlineHtml
 *              指定したHTMLデータがヘッドラインの一覧であるかを判定する
 * @param[in]   strHtml 判定対象のHTML
 * @return  判定結果
 * @retval  true    ヘッドライン一覧HTML
 * @retval  false   非ヘッドライン一覧HTML
 */
bool ThreadListLoader::isHeadlineHtml(const QString& strHtml) const {
    static constexpr auto STR_THREAD_HTML_KEYWORDS {R"(<meta\sname.*?=.*?"keywords".*?>)"};
    QRegularExpression reThreadHtmlKeywords {STR_THREAD_HTML_KEYWORDS, QRegularExpression::CaseInsensitiveOption};
    return strHtml.indexOf(reThreadHtmlKeywords) >= 0;
}

/**
 * @brief       ThreadListLoader::loadNonHeadline
 *              非ヘッドラインスレッド一覧のHTMLを取得する
 * @param[in]   strThreadListHtmlUrl    スレッド一覧URL(非ヘッドラインの一覧を取得するURLを含むページのURL)
 * @param[in]   util                      ユーティリティオブジェクト
 * @return  取得した非ヘッドラインスレッド一覧のHTMLデータ
 * @note    取得に失敗した場合は空のQSｔringを返す
 */
QString ThreadListLoader::loadNonHeadline(const QString &strThreadListHtmlUrl, HBBSUtil& util) {
    QString strNoHeadlineUrl = strThreadListHtmlUrl;
    strNoHeadlineUrl.replace("http", "https");
    strNoHeadlineUrl += "subback.html";
    // 全件ページのHTMLを取得する
    return util.receiveHtml(_pManager, strNoHeadlineUrl);
}

}   // end of namespace
