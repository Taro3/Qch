#ifndef THREADLISTMODELLOADER_H
#define THREADLISTMODELLOADER_H

#include <QObject>
#include <QDateTime>

class QStandardItemModel;
class QNetworkAccessManager;

namespace threadlist {

/**
 * @brief The ThreadListModelLoader class
 */
class ThreadListModelLoader : public QObject
{
    Q_OBJECT
public:
    ThreadListModelLoader(QObject *parent = nullptr);
    QStandardItemModel *model(const QString strUrl, QNetworkAccessManager *pManager);
    QStandardItemModel *model(const QString strFilename);

signals:

private:
    bool loadThreadHtml(QString *html, bool *isHeadline, QNetworkAccessManager *pManager, const QString strUrl);
    void extractHeadlineData(const QRegularExpressionMatch &remThread, QDateTime *cDT, QString *strThreadUrl, QString *strTitle) const;
    void extractNonHeadlineData(const QRegularExpressionMatch &remThread, QString *strThreadUrl, QString *strTitle, int *nThreadNumber, const QString strUrl) const;
    QString arrangeTitle(const QString strTitleHtml, const bool isHeadline);
    void addThreadItem(QStandardItemModel *pModel, const int nThreadNumber, const QString strTitle, const QDateTime cDT, const QString strThreadUrl) const;
};

}   // end of namespace

#endif // THREADLISTMODELLOADER_H
