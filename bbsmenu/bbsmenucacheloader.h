/**
 * @file    bbsmenucacheloader.h
 *
 * @brief   板一覧(メニュー一覧)キャッシュデータ読み込みクラスヘッダ
 *
 * @author  Taro.Yamada
 *
 * @date    2019/09/13
 */

#ifndef BBSMENUCACHELOADER_H
#define BBSMENUCACHELOADER_H

#include <QObject>
#include <QXmlStreamReader>

#include "bbsmenuloaderbase.h"

class QNetworkAccessManager;
class QStandardItem;

namespace bbsmenu {

/**
 * @brief The BbsMenuCacheLoader class
 */
class BbsMenuCacheLoader : public BbsMenuLoaderBase {
    Q_OBJECT
public:
    explicit BbsMenuCacheLoader(QObject *parent = nullptr);
    QStandardItemModel *loadBbsMenu();

signals:

public slots:

private:
    QStandardItemModel *readCategories(QXmlStreamReader &xml) const noexcept;
    QStandardItem *createThreadList(QXmlStreamReader &xml) const;
    void readThreadData(QXmlStreamReader &xml, QStandardItem * const pItem) const;
    QPair<QString, QString> readTitleAndUrl(QXmlStreamReader &xml) const;
};

}   // end of namespace

#endif // BBSMENUCACHELOADER_H
