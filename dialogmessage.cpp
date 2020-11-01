#include <QTextCodec>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QNetworkCookie>

#include "dialogmessage.h"
#include "ui_dialogmessage.h"
#include "settings.h"

/**
 * @brief       DialogMessage::DialogMessage
 *              コンストラクタ
 * @param[in]   parent  親ウィジェット
 * @return      なし
 */
DialogMessage::DialogMessage(QWidget *parent)
    : QDialog{parent}
    , ui{new Ui::DialogMessage}
    , m_canClose{false}
    , m_strHostName{""}
    , m_strBbsName{""}
    , m_strThreadKey{""}
{
    ui->setupUi(this);
}

/**
 * @brief   DialogMessage::~DialogMessage
 *          デストラクタ
 */
DialogMessage::~DialogMessage()
{
    delete ui;
}

/**
 * @brief       DialogMessage::setServerInfo
 *              サーバ情報設定
 * @param[in]   strHostName     ホスト名
 * @param[in]   strBbsName      BBS名
 * @param[in]   strThreadKey    スレッドキー
 * @return      なし
 */
void DialogMessage::setServerInfo(const QString strHostName, const QString strBbsName, const QString strThreadKey)
{
    m_strHostName  = strHostName;
    m_strBbsName   = strBbsName;
    m_strThreadKey = strThreadKey;
}

/**
 * @brief       DialogMessage::makePostParameter
 *              POSTパラメータ作成
 * @param[in]   isSecondPost    書込回数(書き込むためには確認を含めて2回書き込む必要がある)
 * @arg         false           1回目書込
 * @arg         true            2回目書込
 * @return      POSTデータ
 */
QByteArray DialogMessage::makePostParameter(const bool isSecondPost)
{
    QByteArray baPostData;
    auto pShiftJisCodec = QTextCodec::codecForName("Shift-JIS");
    auto strName = ui->lineEditName->text();
    auto strMailAddress = ui->lineEditMailAddress->text();
    auto strMessage = ui->textEditMessage->toPlainText();

    if (!isSecondPost) {
        auto strSubmit = "書き込む";
        baPostData  = "bbs="  + m_strBbsName.toLatin1()
                + "&key="     + m_strThreadKey.toLatin1()
                + "&time=1"
                + "&submit="  + pShiftJisCodec->fromUnicode(strSubmit)
                + "&FROM="    + pShiftJisCodec->fromUnicode(strName)
                + "&mail="    + pShiftJisCodec->fromUnicode(strMailAddress)
                + "&MESSAGE=" + pShiftJisCodec->fromUnicode(strMessage);
    } else {
        auto strSubmit = "上記全てを承諾して書き込む";
        baPostData  = "bbs="  + m_strBbsName.toLatin1()
                + "&key="     + m_strThreadKey.toLatin1()
                + "&time=1"
                + "&submit="  + pShiftJisCodec->fromUnicode(strSubmit)
                + "&FROM="    + pShiftJisCodec->fromUnicode(strName)
                + "&mail="    + pShiftJisCodec->fromUnicode(strMailAddress)
                + "&MESSAGE=" + pShiftJisCodec->fromUnicode(strMessage)
                + "&suka=pontan";
    }

    return baPostData;
}

/**
 * @brief DialogMessage::postWriteData
 * @param isSecondWrite
 * @param cRequest
 * @param strData
 * @return
 */
bool DialogMessage::postWriteData(const bool isSecondWrite, QNetworkRequest &cRequest, QString &strData)
{
    static QList<QNetworkCookie> lstCookies;
    QNetworkAccessManager cManager;

    makePostHeader(cRequest, lstCookies);

    auto pReply = cManager.post(cRequest, makePostParameter(isSecondWrite));
    while (pReply->isRunning()) {
        qApp->processEvents();
    }
    if (pReply->error() != QNetworkReply::NoError) {
        pReply->close();
        strData = pReply->errorString();
        return false;
    }
    auto baData = pReply->readAll();
    auto pHtmlCodec = QTextCodec::codecForHtml(baData);
    strData = pHtmlCodec->toUnicode(baData);
    if (strData.indexOf("ERROR") >= 0) {
        pReply->close();
        return false;
    }
    QList<QNetworkCookie> lstSetCookie;
    lstSetCookie = pReply->header(QNetworkRequest::SetCookieHeader).value<QList<QNetworkCookie>>();
    lstCookies = lstSetCookie;

    return true;
}

/**
 * @brief DialogMessage::makePostHeader
 * @param cPostRequest
 * @param lstCookies
 */
void DialogMessage::makePostHeader(QNetworkRequest &cPostRequest, const QList<QNetworkCookie> &lstCookies)
{
    cPostRequest.setUrl(QUrl("http://" + m_strHostName + "/test/bbs.cgi"));
    cPostRequest.setRawHeader("Referer", QByteArray("http://") + m_strHostName.toLatin1() + "/"
                              + m_strBbsName.toLatin1() + "/");
    cPostRequest.setHeader(QNetworkRequest::UserAgentHeader, "Monazilla/1.00");
    cPostRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    if (!lstCookies.isEmpty()) {
        cPostRequest.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue(lstCookies));
    }
}

/**
 * @brief       DialogMessage::on_buttonBox_clicked
 *              ボタンクリックイベントハンドラ
 * @param[in]   button  クリックされたボタン
 * @return      なし
 */
void DialogMessage::on_buttonBox_clicked(QAbstractButton *button)
{
    if (button != ui->buttonBox->button(QDialogButtonBox::Ok)) {
        reject();
        return;
    }

    QNetworkRequest cRequest;
    QString strReplyData;

    if (!postWriteData(false, cRequest, strReplyData)) {
        QMessageBox::warning(this, tr("Post failed"), strReplyData);
        return;
    }

    Settings cSettings;
    if (cSettings.value(Settings::SETTING_FIRST_POST).compare("yes", Qt::CaseInsensitive) == 0) {
        if (QMessageBox::question(this, tr("Write confirm"), strReplyData) != QMessageBox::Yes) {
            return;
        }
        cSettings.setValue(Settings::SETTING_FIRST_POST, "no");
    }

    if (!postWriteData(true, cRequest, strReplyData)) {
        QMessageBox::warning(this, tr("Post failed"), strReplyData);
        return;
    }

    accept();
}

/**
 * @brief DialogMessage::on_checkBoxSage_toggled
 * @param checked
 */
void DialogMessage::on_checkBoxSage_toggled(bool checked)
{
    ui->lineEditMailAddress->setEnabled(!checked);
    ui->lineEditMailAddress->setText(checked ? "sage" : "");
}
