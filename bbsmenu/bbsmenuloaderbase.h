/**
 * @file    bbsmenuloaderbase.h
 *
 * @brief   掲示板メニューローダ基底クラスヘッダ
 *
 * @author  Taro.Yamada
 *
 * @date    2019/09/13
 */

#ifndef BBSMENULOADERBASE_H
#define BBSMENULOADERBASE_H

#include <QObject>

class QStandardItemModel;

namespace bbsmenu {

/**
 * @brief The BbsMenuLoaderBase class
 */
class BbsMenuLoaderBase : public QObject {
    Q_OBJECT
public:
    explicit BbsMenuLoaderBase(QObject *parent = nullptr);
    virtual QStandardItemModel *loadBbsMenu() = 0;
    virtual ~BbsMenuLoaderBase();

signals:

public slots:
};

}   // end of namespace

#endif // BBSMENULOADERBASE_H
