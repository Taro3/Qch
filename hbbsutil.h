#ifndef HBBSUTIL_H
#define HBBSUTIL_H

#include <QObject>
#include <QDateTime>

class QNetworkAccessManager;

typedef struct _POST_DATA {
    int       nPostId;              //!< id値(ID)
    QString   strDataDate;          //!< data-date文字列
    QString   strDataUserId;        //!< data-userid文字列
    int       nDataId;              //!< data-id値
//    int       nPostNumber;          //!< 書込番号
    QString   strName;              //!< 書込者名(mailtoを含む)
    QDateTime dtPostedDateTime;     //!< 書込日時
//    QString   strMail;              //!< 書込者メールアドレス
    QString   strMessage;           //!< 書込メッセージ
    QString   strHtml;              //!< 書込HTML
} POST_DATA;

/**
 * @brief The HBBSUtil class
 */
class HBBSUtil : public QObject {
    Q_OBJECT
public:

    explicit            HBBSUtil(QObject *parent = nullptr);
    virtual             ~HBBSUtil();
    QString             loadThreadHtml(const QString strThreadUrl);
    bool                postData(const QString strThreadHtml, QList<POST_DATA> &lstPostData);
    QString receiveHtml(QNetworkAccessManager *pManager, const QString strHtmlUrl);

signals:

public slots:

private:
    QList<POST_DATA> m_lstPostData;
    QString          m_strViewFontName;

    QString     postedHtml(const QString strHtml, const int nPostNumber, int &nStartIndex, const bool isBbsPinkThread);
    bool        postLineData(const QString strPostHtml, int &nPostId, QString &strDataDate, QString &strDataUserId, int &nDataId, const bool isBbsPinkThread);
    bool        nameLineData(const QString strPostHtml, QString &strName);
    bool        dateLineData(const QString strPostHtml, QDateTime &dtPostedDateTime);
    bool        messageLineData(const QString strPostHtml, QString &strMessage, const bool isBbspinkThread);
    bool        checkBbsPink(const QString strThreadHtml);
    bool        replaceAnchors(QString &strPostedHtml);
};


#endif // HBBSUTIL_H
