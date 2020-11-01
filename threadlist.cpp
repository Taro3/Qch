#include <QtGlobal>
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
#include <QDateTime>

#include "threadlist.h"
#include "settings.h"

//======================================================================================================================
/**
 * @brief       ThreadList::ThreadList
 *              コンストラクタ
 * @param[in]   pManager    ネットワーク接続用オブジェクト
 * @param[in]   parent      親オブジェクトポインタ
 */
ThreadList::ThreadList(QNetworkAccessManager * const pManager, QObject *parent)
    : QObject{parent}
    , m_pcNetworkAccessManager{nullptr}
    , m_pcMenuItem{nullptr}
{
    Q_ASSERT(pManager);
    m_pcNetworkAccessManager = pManager;
}

//======================================================================================================================
/**
 * @brief       ThreadList::threadListModel
 *              BBSメニューのモデル取得
 * @param[in]   strUrl  BBSメニューのURL文字列
 * @return      生成されたBBSメニュー用モデルオブジェクト
 */
QStandardItemModel *ThreadList::threadListModel(const QString strUrl)
{
    auto pThreadListModel = new QStandardItemModel;

    auto isHeadline = true;
    QString strThreadsHtml;
    if (!loadThreads(strUrl, strThreadsHtml, isHeadline)) {
        return pThreadListModel;
    }

    // スレッド抽出用正規表現を設定
    QString STR_THREAD_LINE_RE;
    if (isHeadline) {
        STR_THREAD_LINE_RE = "(\\d{4}/\\d{2}/\\d{2}\\s\\d{2}:\\d{2}:\\d{2})\\s<a\\shref.*?=(.*?)>(.*?)</a>";
    } else {
        STR_THREAD_LINE_RE = "<a\\shref.*?=\"(.*?)/l50\">(\\d+):\\s(.*?)</a>";
    }

    QRegularExpression reThreadLine{STR_THREAD_LINE_RE, QRegularExpression::CaseInsensitiveOption};
    auto nThreadNumber = 1;
    auto nIndex = 0;
    while (true) {
        auto reMatchThread = reThreadLine.match(strThreadsHtml, nIndex);
        if (!reMatchThread.hasMatch()) {
            break;
        }

        QDateTime cDT;
        if (isHeadline) {
            auto strDateTime = reMatchThread.captured(1);
            auto cDT = QDateTime::fromString(strDateTime, "yyyy/MM/dd hh:mm:ss");
        }

        QString strThreadUrl;
        QString strTitle;
        if (isHeadline) {
            strThreadUrl = reMatchThread.captured(2);
            strTitle = reMatchThread.captured(3).trimmed();
        } else {
            strThreadUrl = strUrl;
            int nCgiInsertPos = strThreadUrl.lastIndexOf('/', strThreadUrl.length() - 2);
            strThreadUrl.insert(nCgiInsertPos, "/test/read.cgi");
            strThreadUrl = strThreadUrl + reMatchThread.captured(1);
            strTitle = reMatchThread.captured(3).trimmed();
            nThreadNumber = reMatchThread.captured(2).toInt();
        }

        strTitle.remove(QRegularExpression("<font\\scolor.*?=.*?>", QRegularExpression::CaseInsensitiveOption));
        strTitle.remove(QRegularExpression("</font>", QRegularExpression::CaseInsensitiveOption));
        strTitle.replace("&#169;", "(c)");

        if (!isHeadline) {
            strTitle.remove(QRegularExpression("\\s\\(\\d+\\)"));
        }

        auto pItem = new QStandardItem(strTitle);
        THREAD_TITLE_DATA sData;
        sData.nThreadNumber = nThreadNumber++;
        sData.cThreadCreateedDateTime = cDT;
        sData.strThreadTitle = strTitle;
        sData.strThreadUrl = strThreadUrl;
        QVariant v;
        v.setValue(sData);
        pItem->setData(v);
        pThreadListModel->appendRow(pItem);

        nIndex = reMatchThread.capturedEnd() + 1;
        qDebug() << cDT.toString() << strTitle << " " << strUrl;
    }

    return pThreadListModel;
}

//======================================================================================================================
/**
 * @brief ThreadList::updateThreadListData
 * @param strThreadListUrl
 */
void ThreadList::updateThreadListData(const QString strThreadListUrl)
{
}

//======================================================================================================================
/**
 * @brief ThreadList::loadThreads
 * @param strThreadListHtmlUrl
 * @param strThreadListHtml
 * @param isHeadline
 * @return
 * @todo    引数と戻り値を再検討する参照渡しは使いたくない。構造体にしてわかりやすくすること。
 *          更にQNetworkAccessManagerをローカルで宣言しているが、メンバ変数に持っているはずなのでそれを使用すること。
 */
bool ThreadList::loadThreads(const QString strThreadListHtmlUrl, QString &strThreadListHtml, bool &isHeadline)
{
    isHeadline = true;
    strThreadListHtml.clear();

    // 指定URLのスレッド一覧HTMLを取得する
    QNetworkAccessManager cManager;
    auto pReply = cManager.get(QNetworkRequest(QUrl(strThreadListHtmlUrl)));
    while (pReply->isRunning()) {
        qApp->processEvents();
    }
    if (pReply->error() != QNetworkReply::NetworkError::NoError) {
        qDebug() << pReply->errorString();
        return false;
    }
    auto baThreadsHtml = pReply->readAll();
    pReply->close();
    auto pHtmlCodec = QTextCodec::codecForHtml(baThreadsHtml);
    strThreadListHtml = pHtmlCodec->toUnicode(baThreadsHtml);

    // ヘッドラインチェック
    static const auto STR_THREAD_HTML_KEYWORDS = "<meta\\sname.*?=.*?\"keywords\".*?>";
    QRegularExpression reThreadHtmlKeywords{STR_THREAD_HTML_KEYWORDS, QRegularExpression::CaseInsensitiveOption};
    // ヘッドライン以外の場合はURLを全件のページに変更する
    if (strThreadListHtml.indexOf(reThreadHtmlKeywords) < 0) {
        auto strNoHeadlineUrl = strThreadListHtmlUrl;
        strNoHeadlineUrl.replace("http", "https");
        strNoHeadlineUrl += "subback.html";
        // 全件ページのHTMLを取得する
        pReply = cManager.get(QNetworkRequest(QUrl(strNoHeadlineUrl)));
        while (pReply->isRunning()) {
            qApp->processEvents();
        }
        if (pReply->error() != QNetworkReply::NoError) {
            qDebug() << pReply->errorString();
            return false;
        }
        baThreadsHtml = pReply->readAll();
        pReply->close();
        pHtmlCodec = QTextCodec::codecForHtml(baThreadsHtml);
        strThreadListHtml = pHtmlCodec->toUnicode(baThreadsHtml);
        isHeadline = false;
    }
    // ヘッドラインの場合は取得したHTMLをそのまま使用する

    return true;
}

//======================================================================================================================
/**
 * @brief ThreadList::loadThreadsFromFile
 * @param strThreadListUrl
 * @param clstThreads
 * @return
 */
bool ThreadList::loadThreadsFromFile(const QString strThreadListUrl, QList<THREAD_TITLE_DATA> clstThreads)
{
    static const auto THREAD_LIST_FILE_SUFFIX = ".xml";
    Settings cSettings;

    clstThreads.clear();

    auto nSlashIndex = strThreadListUrl.lastIndexOf('/');
    if (nSlashIndex < 0)
    {
        qDebug() << "Invalid thread list url.";
        return false;
    }
    auto strThreadListFilePath = cSettings.value(Settings::SETTING_DATA_FILE_PATH)
            + strThreadListUrl.right(strThreadListUrl.length() - nSlashIndex) + THREAD_LIST_FILE_SUFFIX;
    QFile cThreadListFile{strThreadListFilePath};
    if (!cThreadListFile.exists()) {
        qDebug() << strThreadListFilePath + " not found.";
        return false;
    }
    if (!cThreadListFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << strThreadListFilePath + " can't open.";
        return false;
    }

    QXmlStreamReader cReader{&cThreadListFile};
    if (!cReader.readNextStartElement()) {
        qDebug() << "Invalid xml format.";
        return false;
    }
    if (cReader.name().compare(QString("thread_list"), Qt::CaseInsensitive) != 0) {
        qDebug() << "Invalid xml format.";
        return false;
    }
    auto nDataCount = cReader.attributes().value("count").toInt();
    for (auto i = 0; i < nDataCount; ++i) {
        if (!cReader.readNextStartElement()) {
            clstThreads.clear();
            qDebug() << "Invalid xml format.";
            return false;
        }
        if (cReader.name().compare(QString("data"), Qt::CaseInsensitive) != 0) {
            clstThreads.clear();
            qDebug() << "Invalid xml format.";
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

//======================================================================================================================
/**
 * @brief ThreadList::loadThreadsFromWeb
 * @param strThreadListUrl
 * @param clstThreads
 * @return
 */
bool ThreadList::loadThreadsFromWeb(const QString strThreadListUrl, QList<THREAD_TITLE_DATA> clstThreads)
{
    clstThreads.clear();

    // スレッド一覧HTMLを取得する
    auto strThreadListHtml = fetchThreadList(strThreadListUrl);
    if (strThreadListHtml.isEmpty()) {
        return false;
    }

    // ヘッドラインチェック
    static const auto STR_THREAD_HTML_KEYWORDS = "<meta\\sname.*?=.*?\"keywords\".*?>";
    QRegularExpression reThreadHtmlKeywords{STR_THREAD_HTML_KEYWORDS, QRegularExpression::CaseInsensitiveOption};
    // ヘッドライン以外の場合はURLを全件のページに変更する
    auto isHeadline = checkHeadline(strThreadListHtml);
    if (isHeadline) {
        strThreadListHtml = fetchNonHeadlineThreadList(strThreadListUrl);
        if (strThreadListHtml.isEmpty()) {
            return false;
        }
    } else {
        // ヘッドラインの場合は取得したHTMLをそのまま使用する
    }

    // スレッドデータ行処理
    static const QString STR_THREAD_LINE_RE[] = {
        "(\\d{4}/\\d{2}/\\d{2}\\s\\d{2}:\\d{2}:\\d{2})\\s<a\\shref.*?=(.*?)>(.*?)</a>",
        "<a\\shref.*?=\"(.*?)/l50\">(\\d+):\\s(.*?)</a>",
    };
    auto strThreadLineRe = STR_THREAD_LINE_RE[isHeadline ? 0 : 1];

    QRegularExpression reThreadLine{strThreadLineRe, QRegularExpression::CaseInsensitiveOption};
    auto nThreadNumber = 1;
    auto nIndex = 0;
    while (true) {
        // スレッド行取得
        auto reMatchThread = reThreadLine.match(strThreadListHtml, nIndex);
        if (!reMatchThread.hasMatch()) {
            break;
        }

        // 日時取得
        QDateTime cDT;
        if (isHeadline) {
            cDT = threadDateTime(reMatchThread.captured(1));
            if (!cDT.isValid()) {
                qDebug() << QString(__FILE__) + " (" + QString(__LINE__) + ") : Invalid Datetime.";
            }
        }

        // スレッド番号、タイトル、URL取得
        QString strThreadUrl;
        QString strTitle;
        if (!threadTitleAndUrl(isHeadline, reMatchThread, strTitle, strThreadUrl, nThreadNumber)) {
            qDebug() << QString(__FILE__) + " (" + QString(__LINE__) + ") : Invalid title or url.";
        }

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

//======================================================================================================================
/**
 * @brief       ThreadList::fetchThreadList
 *              Webからスレッド一覧のHTMLを取得する
 * @param[in]   strThreadListUrl    スレッド一覧HTMLのURL
 * @return      取得したスレッド一覧のHTML文字列
 * @note        取得に失敗した場合はから文字列を返す
 */
QString ThreadList::fetchThreadList(const QString strThreadListUrl)
{
    auto pReply = m_pcNetworkAccessManager->get(QNetworkRequest(QUrl(strThreadListUrl)));
    while (pReply->isRunning()) {
        qApp->processEvents();
    }
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

//======================================================================================================================
/**
 * @brief       ThreadList::checkHeadline
 *              スレッド一覧のHTMLからヘッドラインのデータであるかをチェックする
 * @param[in]   strThreadListHtml
 * @return      チェック結果
 * @retval      true    ヘッドラインデータ
 * @retval      false   非ヘッドラインデータ
 */
bool ThreadList::checkHeadline(const QString strThreadListHtml)
{
    static const auto STR_THREAD_HTML_KEYWORDS = "<meta\\sname.*?=.*?\"keywords\".*?>";
    static const QRegularExpression reThreadHtmlKeywords{STR_THREAD_HTML_KEYWORDS
                                                         , QRegularExpression::CaseInsensitiveOption};

    return strThreadListHtml.indexOf(reThreadHtmlKeywords) >= 0;    // 上記RegExpがマッチする場合はヘッドライン
}

//======================================================================================================================
/**
 * @brief       ThreadList::fetchNonHeadlineThreadList
 *              非ヘッドラインのスレッド一覧HTML取得
 * @param[in]   strThredListUrl スレッド一覧HTMLのURL
 * @return      取得したスレッド一覧HTMLデータ
 * @note        取得に失敗した場合は空文字列を返す
 */
QString ThreadList::fetchNonHeadlineThreadList(const QString strThreadListUrl)
{
    // URLを変換する
    auto strNoHeadlineUrl = strThreadListUrl;
    strNoHeadlineUrl.replace("http", "https");
    strNoHeadlineUrl += "subback.html";

    // 全件ページのHTMLを取得する
    auto pReply = m_pcNetworkAccessManager->get(QNetworkRequest(QUrl(strNoHeadlineUrl)));
    while (pReply->isRunning()) {
        qApp->processEvents();
    }
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

//======================================================================================================================
/**
 * @brief       ThreadList::threadDateTime
 *              スレッドデータ行から作成日時を取得する
 * @param[in]   strDateTime 日時文字列
 * @return      取得した日時
 *              エラー発生時は無効なQDateTimeを返す
 * @note        日時データはヘッドラインにしか存在しない
 */
QDateTime ThreadList::threadDateTime(const QString strDateTime)
{
    auto cDT = QDateTime::fromString(strDateTime, "yyyy/MM/dd hh:mm:ss");

    return cDT;
}

//======================================================================================================================
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

    strTitle.remove(QRegularExpression("<font\\scolor.*?=.*?>", QRegularExpression::CaseInsensitiveOption));
    strTitle.remove(QRegularExpression("</font>", QRegularExpression::CaseInsensitiveOption));
    strTitle.replace("&#169;", "(c)");                          /// @todo 置換リスト処理を追加する

    if (!isHeadline) {
        strTitle.remove(QRegularExpression("\\s\\(\\d+\\)"));   /// @todo レス数取得処理を入れる
    }

    return true;
}
