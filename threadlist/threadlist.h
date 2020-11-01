#ifndef THREADLIST_H
#define THREADLIST_H

#include <QObject>
#include <QDateTime>
#include <QRegularExpressionMatch>

#include "TLDefine.h"

class QNetworkAccessManager;
class QStandardItemModel;
class QStandardItem;

namespace threadlist {

/**
 * @brief The ThreadList class
 */
class ThreadList : public QObject {
    Q_OBJECT
public:
    ThreadList(QNetworkAccessManager * const pManager, QObject *parent = nullptr);
    QStandardItemModel *threadListModel(const QString strUrl);
    void updateThreadListData(const QString strThreadListUrl);

signals:

public slots:

private:
    QNetworkAccessManager *_pcNetworkAccessManager; //!< ネットワークアクセス用オブジェクト

    bool loadThreadsFromFile(const QString strThreadListUrl, QList<THREAD_TITLE_DATA> clstThreads);
    bool loadThreadsFromWeb(const QString strThreadListUrl, QList<THREAD_TITLE_DATA> clstThreads);
    const QString fetchThreadList(const QString strThreadListUrl);
    bool checkHeadline(const QString strThreadListHtml);
    const QString fetchNonHeadlineThreadList(const QString strThreadListUrl);
    const QDateTime threadDateTime(const QString strDateTime);
    bool threadTitleAndUrl(const bool isHeadline, const QRegularExpressionMatch &reMatchThread, QString &strTitle, QString &strUrl, int &nThreadNumber);
};

}   // end of namespace

#endif // THREADLIST_H
