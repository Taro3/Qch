#ifndef DATACACHE_H
#define DATACACHE_H

#include <QObject>

/**
 * @brief The DataCache class
 */
class DataCache : public QObject
{
    Q_OBJECT

public:
    explicit DataCache(QObject *parent = nullptr);

signals:

public slots:

private:
    QString m_strDataFilePath;
};

#endif // DATACACHE_H
