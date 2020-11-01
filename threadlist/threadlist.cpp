#include <QStandardItemModel>
#include <QRegularExpression>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QXmlStreamReader>
#include <QFileInfo>

#include "threadlist.h"
#include "settings.h"
#include "threadlistloader.h"
#include "qchexception.h"
#include "common.h"
#include "threadlistmodelloader.h"

namespace threadlist {

/**
 * @brief       ThreadList::ThreadList
 *              コンストラクタ
 * @param[in]   pManager    ネットワーク接続用オブジェクト
 * @param[in]   parent      親オブジェクトポインタ
 * @return      なし
 */
ThreadList::ThreadList(QNetworkAccessManager * const pManager, QObject *parent) : QObject {parent}
  , _pcNetworkAccessManager {pManager}
{
    Q_ASSERT(pManager);
}

/**
 * @brief       ThreadList::threadListModel
 *              BBSメニューのモデル取得
 * @param[in]   strUrl  BBSメニューのURL文字列
 * @return      生成されたスレッド一覧用モデルオブジェクト
 *              問題が発生した場合は空のQStandardItemModelを返す
 */
QStandardItemModel *ThreadList::threadListModel(const QString strUrl) {
    ThreadListModelLoader tlml;
    return tlml.model(strUrl, _pcNetworkAccessManager);
}

/**
 * @brief       ThreadList::updateThreadListData
 *              スレッドリストの更新処理
 *              TBD
 * @param[in]   strThreadListUrl    スレッドリストURL
 */
void ThreadList::updateThreadListData(const QString strThreadListUrl) {
    Q_UNUSED(strThreadListUrl)
    /// @todo 実装検討
}

/**
 * @brief       ThreadList::loadThreadsFromFile
 *              スレッドリストのファイルからの読込処理
 * @param[in]   strThreadListUrl    スレッドリストのURL
 * @param[out]  clstThreads         読み込んだスレッドのリスト
 * @return  処理結果
 */
bool ThreadList::loadThreadsFromFile(const QString strThreadListUrl, QList<THREAD_TITLE_DATA> clstThreads) {
    static const QString THREAD_LIST_FILE_SUFFIX = ".xml";
    Settings cSettings;

    clstThreads.clear();

    QFileInfo fi{strThreadListUrl};
    QString filename = fi.fileName();
    if (filename.isEmpty()) {
        LOGGING("Invalid thread list url: " + strThreadListUrl)
        return false;
    }
    filename += THREAD_LIST_FILE_SUFFIX;
    QString strThreadListFilePath = cSettings.value(Settings::eSETTING_NAME::SETTING_DATA_FILE_PATH) + "/" + filename;
    QFile cThreadListFile{strThreadListFilePath};
    if (!cThreadListFile.exists()) {
        LOGGING(strThreadListFilePath + " not found.");
        return false;
    }
    if (!cThreadListFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOGGING(strThreadListFilePath + " can't open.");
        return false;
    }

    QXmlStreamReader cReader{&cThreadListFile};
    if (!cReader.readNextStartElement()) {
        LOGGING("Invalid xml format.");
        return false;
    }
    if (cReader.name().compare(QString("thread_list"), Qt::CaseInsensitive) != 0) {
        LOGGING("Invalid xml format.");
        return false;
    }
    int nDataCount = cReader.attributes().value("count").toInt();
    for (int i = 0; i < nDataCount; ++i) {
        if (!cReader.readNextStartElement()) {
            clstThreads.clear();
            LOGGING("Invalid xml format.");
            return false;
        }
        if (cReader.name().compare(QString("data"), Qt::CaseInsensitive) != 0) {
            clstThreads.clear();
            LOGGING("Invalid xml format.");
            return false;
        }
        THREAD_TITLE_DATA data;
        data.nThreadNumber = cReader.attributes().value("number").toInt();
        data.cThreadCreateedDateTime = QDateTime::fromString(cReader.attributes().value("date").toString());
        data.strThreadUrl = cReader.attributes().value("url").toString();
        data.strThreadTitle = cReader.attributes().value("title").toString();
        data.nMessageCount = cReader.attributes().value("message_count").toInt();
        clstThreads.append(data);
    }
    cThreadListFile.close();

    return true;
}

/**
 * @brief ThreadList::loadThreadsFromWeb
 * @param strThreadListUrl
 * @param clstThreads
 * @return
 */
bool ThreadList::loadThreadsFromWeb(const QString strThreadListUrl, QList<THREAD_TITLE_DATA> clstThreads) {
    clstThreads.clear();

    // スレッド一覧HTMLを取得する
    auto strThreadListHtml = fetchThreadList(strThreadListUrl);
    if (strThreadListHtml.isEmpty())
        return false;

    // ヘッドラインチェック
    static const auto STR_THREAD_HTML_KEYWORDS = R"(<meta\sname.*?=.*?"keywords".*?>)";
    QRegularExpression reThreadHtmlKeywords{STR_THREAD_HTML_KEYWORDS, QRegularExpression::CaseInsensitiveOption};
    // ヘッドライン以外の場合はURLを全件のページに変更する
    auto isHeadline = checkHeadline(strThreadListHtml);
    if (isHeadline) {
        strThreadListHtml = fetchNonHeadlineThreadList(strThreadListUrl);
        if (strThreadListHtml.isEmpty())
            return false;
    } else {
        // ヘッドラインの場合は取得したHTMLをそのまま使用する
    }

    // スレッドデータ行処理
    static const QString STR_THREAD_LINE_RE[] = {
        R"((\d{4}/\d{2}/\d{2}\s\d{2}:\d{2}:\d{2})\s<a\shref.*?=(.*?)>(.*?)</a>)",
        R"(<a\shref.*?="(.*?)/l50">(\d+):\s(.*?)</a>)",
    };
    auto strThreadLineRe = STR_THREAD_LINE_RE[isHeadline ? 0 : 1];

    QRegularExpression reThreadLine{strThreadLineRe, QRegularExpression::CaseInsensitiveOption};
    auto nThreadNumber = 1;
    auto nIndex = 0;
    while (true) {
        // スレッド行取得
        auto reMatchThread = reThreadLine.match(strThreadListHtml, nIndex);
        if (!reMatchThread.hasMatch())
            break;

        // 日時取得
        QDateTime cDT;
        if (isHeadline) {
            cDT = threadDateTime(reMatchThread.captured(1));
            if (!cDT.isValid())
                qDebug() << QString(__FILE__) + " (" + QString(__LINE__) + ") : Invalid Datetime.";
        }

        // スレッド番号、タイトル、URL取得
        QString strThreadUrl;
        QString strTitle;
        if (!threadTitleAndUrl(isHeadline, reMatchThread, strTitle, strThreadUrl, nThreadNumber))
            qDebug() << QString(__FILE__) + " (" + QString(__LINE__) + ") : Invalid title or url.";

        // データ追加
        THREAD_TITLE_DATA sData;
        sData.nThreadNumber = nThreadNumber++;
        sData.cThreadCreateedDateTime = cDT;
        sData.strThreadTitle = strTitle;
        sData.strThreadUrl = strThreadUrl;
        clstThreads.append(sData);

        // 次検索位置更新
        nIndex = reMatchThread.capturedEnd() + 1;
    }

    return true;
}

/**
 * @brief       ThreadList::fetchThreadList
 *              Webからスレッド一覧のHTMLを取得する
 * @param[in]   strThreadListUrl    スレッド一覧HTMLのURL
 * @return      取得したスレッド一覧のHTML文字列
 * @note        取得に失敗した場合はから文字列を返す
 */
const QString ThreadList::fetchThreadList(const QString strThreadListUrl) {
    auto pReply = _pcNetworkAccessManager->get(QNetworkRequest(QUrl(strThreadListUrl)));
    while (pReply->isRunning())
        qApp->processEvents();
    if (pReply->error() != QNetworkReply::NoError) {
        qDebug() << QString(__FILE__) + " (" + QString(__LINE__) + ") : " + pReply->errorString();
        return "";
    }
    auto baThreadListHtml = pReply->readAll();
    pReply->close();
    auto pCodec = QTextCodec::codecForHtml(baThreadListHtml);
    auto strThreadListHtml = pCodec->toUnicode(baThreadListHtml);

    return strThreadListHtml;
}

/**
 * @brief       ThreadList::checkHeadline
 *              スレッド一覧のHTMLからヘッドラインのデータであるかをチェックする
 * @param[in]   strThreadListHtml
 * @return      チェック結果
 * @retval      true    ヘッドラインデータ
 * @retval      false   非ヘッドラインデータ
 */
bool ThreadList::checkHeadline(const QString strThreadListHtml) {
    static const auto STR_THREAD_HTML_KEYWORDS = R"(<meta\sname.*?=.*?"keywords".*?>)";
    static const QRegularExpression reThreadHtmlKeywords{STR_THREAD_HTML_KEYWORDS
                                                         , QRegularExpression::CaseInsensitiveOption};

    return strThreadListHtml.indexOf(reThreadHtmlKeywords) >= 0;    // 上記RegExpがマッチする場合はヘッドライン
}

/**
 * @brief       ThreadList::fetchNonHeadlineThreadList
 *              非ヘッドラインのスレッド一覧HTML取得
 * @param[in]   strThredListUrl スレッド一覧HTMLのURL
 * @return      取得したスレッド一覧HTMLデータ
 * @note        取得に失敗した場合は空文字列を返す
 */
const QString ThreadList::fetchNonHeadlineThreadList(const QString strThreadListUrl) {
    // URLを変換する
    auto strNoHeadlineUrl = strThreadListUrl;
    strNoHeadlineUrl.replace("http", "https");
    strNoHeadlineUrl += "subback.html";

    // 全件ページのHTMLを取得する
    auto pReply = _pcNetworkAccessManager->get(QNetworkRequest(QUrl(strNoHeadlineUrl)));
    while (pReply->isRunning())
        qApp->processEvents();
    if (pReply->error() != QNetworkReply::NoError) {
        qDebug() << QString(__FILE__) + " (" + QString(__LINE__) + ") : " + pReply->errorString();
        return "";
    }
    auto baThreadsHtml = pReply->readAll();
    pReply->close();
    auto pHtmlCodec = QTextCodec::codecForHtml(baThreadsHtml);
    auto strThreadListHtml = pHtmlCodec->toUnicode(baThreadsHtml);

    return strThreadListHtml;
}

/**
 * @brief       ThreadList::threadDateTime
 *              スレッドデータ行から作成日時を取得する
 * @param[in]   strDateTime 日時文字列
 * @return      取得した日時
 *              エラー発生時は無効なQDateTimeを返す
 * @note        日時データはヘッドラインにしか存在しない
 */
const QDateTime ThreadList::threadDateTime(const QString strDateTime) {
    auto cDT = QDateTime::fromString(strDateTime, "yyyy/MM/dd hh:mm:ss");

    return cDT;
}

/**
 * @brief       ThreadList::threadTitleAndUrl
 *              スレッドタイトル、スレッドURL、スレッド番号の取得
 * @param[in]   isHeadline      ヘッドラインフラグ
 * @param[in]   reMatchThread   検索一致オブジェクト
 * @param[out]  strTitle        取得したタイトル文字列
 * @param[out]  strUrl          取得したURL文字列
 * @param[out]  nThreadNumber   取得したスレッド番号
 * @return      処理結果
 * @retval      true    正常終了
 * @retval      false   エラー
 */
bool ThreadList::threadTitleAndUrl(const bool isHeadline, const QRegularExpressionMatch &reMatchThread
                                   , QString &strTitle, QString &strUrl, int &nThreadNumber)
{
    if (isHeadline) {
        strUrl = reMatchThread.captured(2);
        strTitle = reMatchThread.captured(3).trimmed();
    } else {
        auto nCgiInsertPos = strUrl.lastIndexOf('/', strUrl.length() - 2);
        strUrl.insert(nCgiInsertPos, "/test/read.cgi");
        strUrl = strUrl + reMatchThread.captured(1);
        strTitle = reMatchThread.captured(3).trimmed();
        nThreadNumber = reMatchThread.captured(2).toInt();
    }

    strTitle.remove(QRegularExpression(R"(<font\scolor.*?=.*?>)", QRegularExpression::CaseInsensitiveOption));
    strTitle.remove(QRegularExpression("</font>", QRegularExpression::CaseInsensitiveOption));
    strTitle.replace("&#169;", "(c)");                          /// @todo 置換リスト処理を追加する

    if (!isHeadline)
        strTitle.remove(QRegularExpression(R"(\s\(\d+\))"));   /// @todo レス数取得処理を入れる

    return true;
}

}   // end of namespace
