/**
 * @file    weitemessagedialogmodel.h
 *          メッセージ書き込みダイアログモデルクラスヘッダ
 *
 * @author  Taro.Yamada
 *
 * @date    2019/06/13
 */

#ifndef WRITEMESSAGEDIALOGMODEL_H
#define WRITEMESSAGEDIALOGMODEL_H

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkCookie>

namespace messagedlg {

/**
 * @brief   The WriteMessageDialogModel class
 *          メッセージ書き込み用ダイアログボックスモデルクラス
 */
class WriteMessageDialogModel : public QObject {
    Q_OBJECT
public:
    /**
     * @brief   The WRITE_DATA struct
     *          書き込みデータ定義構造体
     */
    struct WRITE_DATA {
        QString name;
        QString mailAddress;
        QString message;
    };

    explicit WriteMessageDialogModel(QObject *parent = nullptr);
    void setServerInfo(const QString hostName, const QString bbsName, const QString threadKey);
    bool postWriteData(const bool isSecondWrite, QNetworkRequest &request, QString &replyData, const WRITE_DATA &writeData);

signals:
    void beginPost();
    void endPost();

public slots:

private:
    QString _hostName;
    QString _bbsName;
    QString _threadKey;

    QByteArray makePostParameter(const bool isSecondPost, const WRITE_DATA &writeData);
    void makePostHeader(QNetworkRequest &cPostRequest, const QList<QNetworkCookie> &lstCookies);
};

}   // end of namespace

#endif // WRITEMESSAGEDIALOGMODEL_H
