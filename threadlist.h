#ifndef THREADLIST_H
#define THREADLIST_H

#include <QObject>
#include <QDateTime>
#include <QRegularExpressionMatch>

class QNetworkAccessManager;
class QStandardItemModel;
class QStandardItem;

/**
 *  @struct スレッドデータ構造体
 */
typedef struct _THREAD_TITLE_DATA
{
    int       nThreadNumber;            //!< スレッド番号
    QDateTime cThreadCreateedDateTime;  //!< スレッド生成日時
    QString   strThreadUrl;             //!< スレッドURL
    QString   strThreadTitle;           //!< スレッドタイトル
    int       nMessageCount;            //!< レス数
} THREAD_TITLE_DATA;
Q_DECLARE_METATYPE(THREAD_TITLE_DATA)

/**
 * @brief The ThreadList class
 */
class ThreadList : public QObject
{
    Q_OBJECT
public:
    explicit            ThreadList(QNetworkAccessManager * const pManager, QObject *parent = nullptr);
    QStandardItemModel* threadListModel(const QString strUrl);
    void                updateThreadListData(const QString strThreadListUrl);

signals:

public slots:

private:
    QNetworkAccessManager *m_pcNetworkAccessManager;
    QStandardItem         *m_pcMenuItem;

    bool      loadThreads(const QString strThreadListHtmlUrl, QString &strThreadListHtml, bool &isHeadline);
    bool      loadThreadsFromFile(const QString strThreadListUrl, QList<THREAD_TITLE_DATA> clstThreads);
    bool      loadThreadsFromWeb(const QString strThreadListUrl, QList<THREAD_TITLE_DATA> clstThreads);
    QString   fetchThreadList(const QString strThreadListUrl);
    bool      checkHeadline(const QString strThreadListHtml);
    QString   fetchNonHeadlineThreadList(const QString strThreadListUrl);
    QDateTime threadDateTime(const QString strDateTime);
    bool      threadTitleAndUrl(const bool isHeadline, const QRegularExpressionMatch &reMatchThread, QString &strTitle, QString &strUrl, int &nThreadNumber);
};

#endif // THREADLIST_H
