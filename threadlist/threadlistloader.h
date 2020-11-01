#ifndef THREADLISTLOADER_H
#define THREADLISTLOADER_H

#include <QObject>

#include "hbbsutil.h"

class QNetworkAccessManager;

namespace threadlist {

/**
 * @brief The ThreadListLoader class
 */
class ThreadListLoader : public QObject
{
    Q_OBJECT
public:
    ThreadListLoader(QNetworkAccessManager *pManager, QObject *parent = nullptr);
    std::pair<QString, bool> load(const QString strThreadListHtmlUrl);

signals:

public slots:

private:
    QNetworkAccessManager *_pManager;

    bool isHeadlineHtml(const QString& strHtml) const;
    QString loadNonHeadline(const QString& strThreadListHtmlUrl, HBBSUtil &util);
};

}   // end of namespace

#endif // THREADLISTLOADER_H
