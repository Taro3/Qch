#ifndef COMMON_H
#define COMMON_H

#include <QDebug>

#define LOGGING(msg) qDebug() << __FILE__ << __LINE__ << __FUNCTION__ << ":" << (msg);

#endif // COMMON_H
