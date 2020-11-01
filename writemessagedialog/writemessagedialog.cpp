/**
 * @file    dialogmessage.cpp
 *
 * @brief   BBS書込ダイアログクラス実装
 *
 * @author  Taro.Yamada
 *
 * @date    2019/01/31
 */

#include <QTextCodec>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QNetworkCookie>

#include "writemessagedialog.h"
#include "ui_writemessagedialog.h"
#include "settings.h"
#include "writemessagedialogmodel.h"

namespace messagedlg {

/**
 * @brief       DialogMessage::DialogMessage
 *              コンストラクタ
 *              UIオブジェクトの初期化を行う
 * @param[in]   parent  親ウィジェットポインタ
 * @return      なし
 */
WriteMessageDialog::WriteMessageDialog(QWidget *parent) : QDialog{parent}, ui{new Ui::WriteMessageDialog}
  , _pModel{new WriteMessageDialogModel{this}}, _canClose{false}
{
    ui->setupUi(this);
}

/**
 * @brief   DialogMessage::~DialogMessage
 *          デストラクタ
 *          UIオブジェクト破棄処理
 */
WriteMessageDialog::~WriteMessageDialog() {
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
void WriteMessageDialog::setServerInfo(const QString strHostName, const QString strBbsName
                                       , const QString strThreadKey)
{
    _pModel->setServerInfo(strHostName, strBbsName, strThreadKey);
}

/**
 * @brief       DialogMessage::on_buttonBox_clicked
 *              ボタンクリックイベントハンドラ
 * @param[in]   button  クリックされたボタン
 * @return      なし
 * @note        SLOT
 */
void WriteMessageDialog::on_buttonBox_clicked(QAbstractButton *button) {
    if (button != ui->buttonBox->button(QDialogButtonBox::Ok)) {
        reject();
        return;
    }

    QNetworkRequest cRequest;
    QString strReplyData;

    // 書き込みデータ設定
    WriteMessageDialogModel::WRITE_DATA writeData;
    writeData.name = ui->lineEditName->text();
    writeData.mailAddress = ui->lineEditMailAddress->text();
    writeData.message = ui->textEditMessage->toPlainText();

    // 1回目POST
    if (!_pModel->postWriteData(false, cRequest, strReplyData, writeData)) {
        QMessageBox::warning(this, tr("Post failed"), strReplyData);
        return;
    }

    // 初の書き込みの場合は免責関連のメッセージを表示して同意を求める
    Settings cSettings;
    if (cSettings.value(Settings::eSETTING_NAME::SETTING_FIRST_POST).compare("yes", Qt::CaseInsensitive) == 0) {
        if (QMessageBox::question(this, tr("Write confirm"), strReplyData) != QMessageBox::Yes)
            return;
        cSettings.setValue(Settings::eSETTING_NAME::SETTING_FIRST_POST, "no");
    }

    // 2回目POST
    if (!_pModel->postWriteData(true, cRequest, strReplyData, writeData)) {
        QMessageBox::warning(this, tr("Post failed"), strReplyData);
        return;
    }

    accept();
}

/**
 * @brief       DialogMessage::on_checkBoxSage_toggled
 *              sageチェックボックス状態変更イベントハンドラ
 * @param[in]   checked チェック状態
 * @return      なし
 * @note        SLOT
 */
void WriteMessageDialog::on_checkBoxSage_toggled(bool checked) {
    ui->lineEditMailAddress->setEnabled(!checked);
    ui->lineEditMailAddress->setText(checked ? "sage" : "");
}

}   // end of namespace
