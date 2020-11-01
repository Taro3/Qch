#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QApplication>
#include <QTextCodec>
#include <QRegularExpression>

#include "hbbsutil.h"
#include "settings.h"

static const QString BBSMENU_FILE_NAME = "bbsmenu.xml"; //!< bbsmenu保存用ファイル名

/**
 * @brief       HBBSUtil::HBBSUtil
 *              コンストラクタ
 * @param[in]   parent  親オブジェクトポインタ
 */
HBBSUtil::HBBSUtil(QObject *parent) : QObject(parent), m_lstPostData(QList<POST_DATA>()), m_strViewFontName(QString())
{
    Settings cSettings;
    m_strViewFontName = cSettings.value(Settings::eSETTING_NAME::SETTING_VIEW_FONT_NAME);
}

/**
 * @brief   HBBSUtil::~HBBSUtil
 *          デストラクタ
 */
HBBSUtil::~HBBSUtil() {
}

/**
 * @brief HBBSUtil::postString
 * @param strHtml
 * @param nPostNumber
 * @return
 */
QString HBBSUtil::postedHtml(const QString strHtml, const int nPostNumber, int &nStartIndex
                             , const bool isBbsPinkThread)
{
    QString STR_POST_TEXT_RE;
    if (isBbsPinkThread)
        STR_POST_TEXT_RE = "<dl class=\"post\" id=\"%1\".*?</dl>";
    else
        STR_POST_TEXT_RE = "<div class=\"post\" id=\"%1\".*?</div><br>";

    QRegularExpression rePostText{STR_POST_TEXT_RE.arg(nPostNumber)
                , QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption};
    QRegularExpressionMatch cMatchPost = rePostText.match(strHtml, nStartIndex);

    if (!cMatchPost.hasMatch())
        return "";

    auto strPostText = cMatchPost.captured();
    nStartIndex += strPostText.length();

    return strPostText;
}

/**
 * @brief HBBSUtil::loadThreadHtml
 * @param strThreadUrl
 * @return
 */
QString HBBSUtil::loadThreadHtml(const QString strThreadUrl) {
    QNetworkAccessManager cManager;
    auto pReply = cManager.get(QNetworkRequest(QUrl(strThreadUrl)));
    while (pReply->isRunning())
        qApp->processEvents();
    if (pReply->error() != QNetworkReply::NoError) {
        qDebug() << pReply->errorString();
        return "";
    }
    auto baThreadHtml = pReply->readAll();
    pReply->close();
    auto pCodec = QTextCodec::codecForHtml(baThreadHtml);
    auto strThreadHtml = pCodec->toUnicode(baThreadHtml);

    return strThreadHtml;
}

/**
 * @brief HBBSUtil::postData
 * @param strThreadHtml
 * @param lstPostData
 * @return
 */
bool HBBSUtil::postData(const QString strThreadHtml, QList<POST_DATA> &lstPostData) {
    auto nStartDataId = lstPostData.count() + 1;
    auto nSearchStartIndex = 0;
    auto isBbsPinkThread = checkBbsPink(strThreadHtml);
    for (auto i = nStartDataId; ; ++i) {
        POST_DATA sPostData;

        auto strPostedHtml = postedHtml(strThreadHtml, i, nSearchStartIndex, isBbsPinkThread);

        if (strPostedHtml.isEmpty())
            break;
        if (!postLineData(strPostedHtml, sPostData.nPostId, sPostData.strDataDate, sPostData.strDataUserId
                          , sPostData.nDataId, isBbsPinkThread))
        {
            return false;
        }
        if (!nameLineData(strPostedHtml, sPostData.strName))
            return false;
        if (!dateLineData(strPostedHtml, sPostData.dtPostedDateTime))
            return false;
        if (!messageLineData(strPostedHtml, sPostData.strMessage, isBbsPinkThread))
            return false;

replaceAnchors(strPostedHtml);

//QString strPrependHtml = "<pre>";
//QString strAppendHtml  = "</pre>";
QString strPrependHtml;
QString strAppendHtml;
if (!m_strViewFontName.isEmpty()) {
    strPrependHtml.append(QString("<font face=\"%1\">").arg(m_strViewFontName));
    strAppendHtml.prepend("</font>");
}
strPostedHtml.prepend(strPrependHtml);
strPostedHtml.append(strAppendHtml);
        sPostData.strHtml = strPostedHtml;
        lstPostData.append(sPostData);
    }

    return true;
}

/**
 * @brief       HBBSUtil::receiveHtml
 *              指定URLのHTMLデータを取得する
 * @param[in]   pManager                ネットワークアクセス用オブジェクト
 * @param[in]   strThreadListHtmlUrl    取得URL
 * @return  取得したHTMLデータ
 * @note    取得に失敗した場合はnull文字列を返す
 * @todo    受信処理のスレッド化を行う
 */
QString HBBSUtil::receiveHtml(QNetworkAccessManager *pManager, const QString strHtmlUrl) {
    QString a;
    QNetworkReply *pReply = pManager->get(QNetworkRequest(QUrl(strHtmlUrl)));
    while (pReply->isRunning())
        qApp->processEvents();
    if (pReply->error() != QNetworkReply::NetworkError::NoError) {
        qDebug() << pReply->errorString();
        return nullptr;
    }
    QByteArray baThreadsHtml = pReply->readAll();
    pReply->close();
    QTextCodec *pHtmlCodec = QTextCodec::codecForHtml(baThreadsHtml);
    return pHtmlCodec->toUnicode(baThreadsHtml);
}

bool HBBSUtil::postLineData(const QString strPostHtml, int &nPostId, QString &strDataDate, QString &strDataUserId
                            , int &nDataId, const bool isBbsPinkThread)
{
    QString STR_POST_LINE_RE;
    if (isBbsPinkThread) {
        STR_POST_LINE_RE = "<dl\\sclass.*?=.*?\"post\"\\sid.*?=.*?\"(\\d+)\"\\sdata-date.*?=.*?\"(.*?)\"\\s"
                           "data-userid.*?=.*?\"(.*?)\"\\sdata-id.*?=.*?\"(\\d+)\".*?>";
    } else {
        STR_POST_LINE_RE = "<div\\sclass.*?=.*?\"post\"\\sid.*?=.*?\"(\\d+)\"\\sdata-date.*?=.*?\"(.*?)\"\\s"
                           "data-userid.*?=.*?\"(.*?)\"\\sdata-id.*?=.*?\"(\\d+)\".*?>";
    }
    QRegularExpression rePostLine{STR_POST_LINE_RE, QRegularExpression::CaseInsensitiveOption};

    auto reMatchPostLine = rePostLine.match(strPostHtml);
    if (!reMatchPostLine.hasMatch())
        return false;
    nPostId = reMatchPostLine.captured(1).toInt();
    strDataDate = reMatchPostLine.captured(2);
    strDataUserId = reMatchPostLine.captured(3);
    strDataUserId.remove("ID:", Qt::CaseInsensitive);   // "ID:"を削除
    nDataId = reMatchPostLine.captured(4).toInt();

    return true;
}

bool HBBSUtil::nameLineData(const QString strPostHtml, QString &strName) {
    static const auto STR_NAME_LINE_RE = "<span\\sclass.*?=.*?\"name\".*?>(.*?)</span>";
    QRegularExpression reNameLine{STR_NAME_LINE_RE
                                  , QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption};

    auto reMatchNameLine = reNameLine.match(strPostHtml);
    if (!reMatchNameLine.hasMatch())
        return false;
    strName = reMatchNameLine.captured(1);
    return true;
}

bool HBBSUtil::dateLineData(const QString strPostHtml, QDateTime &dtPostedDateTime) {
    static const auto STR_DATE_LINE_RE = "<span\\sclass.*?=.*?\"date\".*?>(.*?)</span>";
    QRegularExpression reDateLine{STR_DATE_LINE_RE, QRegularExpression::CaseInsensitiveOption};

    auto reMatchDateLine = reDateLine.match(strPostHtml);
    if (!reMatchDateLine.hasMatch())
        return false;
    auto strDate = reMatchDateLine.captured(1);
    strDate.remove(QRegularExpression("\\sID:.*", QRegularExpression::CaseInsensitiveOption));
    dtPostedDateTime = QDateTime::fromString(strDate, "yyyy/MM/dd(ddd) hh:mm:ss.z");
    return true;
}

bool HBBSUtil::messageLineData(const QString strPostHtml, QString &strMessage, const bool isBbspinkThread) {
    QString STR_MESSAGE_LINE_RE;
    if (isBbspinkThread)
        STR_MESSAGE_LINE_RE = "<dd\\sclass.*?=.*?\"thread_in\".*?>(.*?)</dd>";
    else
        STR_MESSAGE_LINE_RE = "<div\\sclass.*?=.*?\"message\".*?>(.*?)</div>";
    QRegularExpression reMessageLine{STR_MESSAGE_LINE_RE
                                     , QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption};

    auto reMatchMessageLine = reMessageLine.match(strPostHtml);
    if (!reMatchMessageLine.hasMatch())
        return false;
    strMessage = reMatchMessageLine.captured(1);
    return true;
}

bool HBBSUtil::checkBbsPink(const QString strThreadHtml) {
    static const auto STR_BBSPINK_CHECK_RE = "<base\\shref.*?=.*?\".*?\\.bbspink\\.com.*?\">";
    if (strThreadHtml.indexOf(QRegularExpression(STR_BBSPINK_CHECK_RE
                                                 , QRegularExpression::CaseInsensitiveOption)) >= 0)
    {
        return true;
    }
    return false;
}

bool HBBSUtil::replaceAnchors(QString &strPostedHtml) {
    static const auto STR_ANCHOR_TAG_RE = "<a\\shref.*?=.*?>.*?</a>";
    QRegularExpression reAnchorTag{STR_ANCHOR_TAG_RE, QRegularExpression::CaseInsensitiveOption};
    auto nSearchStartIndex = 0;
    while (true) {
        qDebug() << strPostedHtml;
        auto reMatchAnchorTag = reAnchorTag.match(strPostedHtml, nSearchStartIndex);
        if (!reMatchAnchorTag.hasMatch())
            break;
        auto strAnchorTag = reMatchAnchorTag.captured();
        qDebug() << strAnchorTag;
        strAnchorTag.remove(QRegularExpression("http://jump.5ch.net/\\?"));

        static const auto STR_NUMBER_ANCHOR_RE = "<a\\shref.*?=.*?\".*?/(\\d+)\"\\srel.*?=.*?>";
        QRegularExpression reNumberAnchorTag{STR_NUMBER_ANCHOR_RE, QRegularExpression::CaseInsensitiveOption};
        auto reMatchNumberAnchorTag = reNumberAnchorTag.match(strAnchorTag);
        if (reMatchNumberAnchorTag.hasMatch()) {
            auto strAnchorNumber = reMatchNumberAnchorTag.captured(1);
            strAnchorTag.replace(reMatchNumberAnchorTag.capturedStart(), reMatchNumberAnchorTag.capturedLength()
                                 , "<a href=\"" + strAnchorNumber + "\">");
        }
        static const auto STR_NUMBER_RANGE_ANCHOR_RE = "<a\\shref.*?=.*?\".*?/(\\d+-\\d+)\"\\srel.*?=.*?>";
        QRegularExpression reNumberRangeAnchorTag{STR_NUMBER_RANGE_ANCHOR_RE
                                                  , QRegularExpression::CaseInsensitiveOption};
        auto reMatchNumberRangeAnchorTag = reNumberRangeAnchorTag.match(strAnchorTag);
        if (reMatchNumberRangeAnchorTag.hasMatch()) {
            auto strAnchorNumber = reMatchNumberRangeAnchorTag.captured(1);
            strAnchorTag.replace(reMatchNumberRangeAnchorTag.capturedStart()
                                 , reMatchNumberRangeAnchorTag.capturedLength()
                                 , "<a href=\"" + strAnchorNumber + "\">");
        }
        qDebug() << strAnchorTag;
        strPostedHtml.replace(reMatchAnchorTag.capturedStart(), reMatchAnchorTag.capturedLength(), strAnchorTag);

        nSearchStartIndex = reMatchAnchorTag.capturedStart() + strAnchorTag.length();
    }
    qDebug() << strPostedHtml;

    return true;
}
