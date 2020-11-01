#ifndef DIALOGMESSAGE_H
#define DIALOGMESSAGE_H

#include <QDialog>
#include <QCloseEvent>
#include <QAbstractButton>
#include <QNetworkRequest>
#include <QNetworkCookie>

namespace Ui {
class DialogMessage;
}

/**
 * @brief The DialogMessage class
 */
class DialogMessage : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMessage(QWidget *parent = 0);
    ~DialogMessage();
    void setServerInfo(const QString strHostName, const QString strBbsName, const QString strThreadKey);

protected:

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_checkBoxSage_toggled(bool checked);

private:
    Ui::DialogMessage *ui;
    bool              m_canClose;
    QString           m_strHostName;
    QString           m_strBbsName;
    QString           m_strThreadKey;

    QByteArray makePostParameter(const bool isSecondPost);
    bool       postWriteData(const bool isSecondWrite, QNetworkRequest &cRequest, QString &strData);
    void       makePostHeader(QNetworkRequest &cPostRequest, const QList<QNetworkCookie> &lstCookies);
};

#endif // DIALOGMESSAGE_H
