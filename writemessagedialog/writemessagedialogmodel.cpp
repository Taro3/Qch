/**
 * @file    weitemessagedialogmodel.cpp
 *          メッセージ書き込みダイアログモデルクラス実装
 *
 * @author  Taro.Yamada
 *
 * @date    2019/06/13
 */

#include <QTextCodec>
#include <QNetworkCookie>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QApplication>

#include "writemessagedialogmodel.h"

namespace messagedlg {

/**
 * @brief       WriteMessageDialogModel::WriteMessageDialogModel
 *              コンストラクタ
 *              メンバ変数の初期化を行う
 * @param[in]   parent  親オブジェクトポインタ
 * @return      なし
 */
WriteMessageDialogModel::WriteMessageDialogModel(QObject *parent) : QObject{parent}, _hostName{QString{}}
  , _bbsName{QString{}}, _threadKey{QString{}}
{
}

/**
 * @brief       WriteMessageDialogModel::setServerInfo
 *              サーバ情報設定処理
 * @param[in]   hostName    掲示板のホスト名
 * @param[in]   bbsName     BBS名
 * @param[in]   threadKey   スレッドキー(スレッド識別用)
 * @return      なし
 */
void WriteMessageDialogModel::setServerInfo(const QString hostName, const QString bbsName, const QString threadKey) {
    _hostName = hostName;
    _bbsName = bbsName;
    _threadKey = threadKey;
}

/**
 * @brief           WriteMessageDialogModel::postWriteData
 *                  掲示板書き込みデータのPOST処理
 * @param[in]       isSecondWrite   2回目書き込みフラグ
 * @param[in]       request         設定済みのQNetworkRequestオブジェクト
 * @param[in,out]   replyData       POST結果取得用文字列
 * @param[in]       postData        POSTするデータ生成用情報構造体
 * @return          処理結果
 * @retval          true    正常終了
 * @retval          false   エラー
 */
bool WriteMessageDialogModel::postWriteData(const bool isSecondWrite, QNetworkRequest &request, QString &replyData
                                            , const WRITE_DATA &postData)
{
    static QList<QNetworkCookie> lstCookies;
    QNetworkAccessManager cManager;

    emit beginPost();

    makePostHeader(request, lstCookies);

    QNetworkReply *pReply = cManager.post(request, makePostParameter(isSecondWrite, postData));
    while (pReply->isRunning())
        qApp->processEvents();
    if (pReply->error() != QNetworkReply::NoError) {
        pReply->close();
        replyData = pReply->errorString();
        qDebug() << "data POST failed. error string=" << replyData;
        return false;
    }
    QByteArray baData = pReply->readAll();
    QTextCodec *pHtmlCodec = QTextCodec::codecForHtml(baData);
    replyData = pHtmlCodec->toUnicode(baData);
    if (replyData.indexOf("ERROR") >= 0) {
        pReply->close();
        qDebug() << "data POST failed.\n" << replyData;
        return false;
    }
    QList<QNetworkCookie> lstSetCookie;
    lstSetCookie = pReply->header(QNetworkRequest::SetCookieHeader).value<QList<QNetworkCookie>>();
    lstCookies = lstSetCookie;

    emit endPost();

    return true;
}

/**
 * @brief       WriteMessageDialogModel::makePostParameter
 * @param[in]   isSecondPost    2回目書き込みフラグ
 * @param[in]   writeData       POSTするデータ生成用情報構造体
 * @return      POSTデータ
 */
QByteArray WriteMessageDialogModel::makePostParameter(const bool isSecondPost, const WRITE_DATA &writeData) {
    QTextCodec *pShiftJisCodec = QTextCodec::codecForName("Shift-JIS");
    QString strSubmit = isSecondPost ? "上記全てを承諾して書き込む" : "書き込む";
    QByteArray baPostData =
            "bbs=" + _bbsName.toLatin1() +
            "&key=" + _threadKey.toLatin1() +
            "&time=1" +
            "&submit=" + pShiftJisCodec->fromUnicode(strSubmit) +
            "&FROM=" + pShiftJisCodec->fromUnicode(writeData.name) +
            "&mail=" + pShiftJisCodec->fromUnicode(writeData.mailAddress) +
            "&MESSAGE=" + pShiftJisCodec->fromUnicode(writeData.message) +
            (isSecondPost ? "" : "&suka=pontan");

    return baPostData;
}

/**
 * @brief           WriteMessageDialogModel::makePostHeader
 * @param[in,out]   cPostRequest    POSTに使用するQNetworkRequestオブジェクト
 * @param[in,out]   lstCookies      POSTに使用するクッキーを保持するQNetworkCookieオブジェクト
 * @return          なし
 */
void WriteMessageDialogModel::makePostHeader(QNetworkRequest &cPostRequest, const QList<QNetworkCookie> &lstCookies) {
    cPostRequest.setUrl(QUrl("http://" + _hostName + "/test/bbs.cgi"));
    cPostRequest.setRawHeader("Referer", QByteArray("http://") + _hostName.toLatin1() + "/"
                              + _bbsName.toLatin1() + "/");
    cPostRequest.setHeader(QNetworkRequest::UserAgentHeader, "Monazilla/1.00");
    cPostRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    if (!lstCookies.isEmpty())
        cPostRequest.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue(lstCookies));
}

}   // end of namespace
