/**
 * @file    writemessagedialog.h
 *
 * @brief   BBS書込ダイアログクラスヘッダ
 *
 * @author  Taro.Yamada
 *
 * @date    2019/01/31
 */

#ifndef WRITEMESSAGEDIALOG_H
#define WRITEMESSAGEDIALOG_H

#include <QDialog>
#include <QCloseEvent>
#include <QAbstractButton>
#include <QNetworkRequest>
#include <QNetworkCookie>

namespace Ui {
class WriteMessageDialog;
}

namespace messagedlg {

class WriteMessageDialogModel;

/**
 * @brief   The DialogMessage class
 *          BBSへの書込を行うダイアログクラス
 */
class WriteMessageDialog : public QDialog {
    Q_OBJECT

public:
    explicit WriteMessageDialog(QWidget *parent = nullptr);
    ~WriteMessageDialog();
    void setServerInfo(const QString strHostName, const QString strBbsName, const QString strThreadKey);

protected:

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_checkBoxSage_toggled(bool checked);

private:
    Ui::WriteMessageDialog *ui;
    WriteMessageDialogModel *_pModel;
    bool _canClose;
};

}   // end of namespace

#endif // WRITEMESSAGEDIALOG_H
