/**
 * @file    bbsmenuwebloader.h
 *
 * @brief   板一覧(メニュー一覧)ウェブデータ読み込みクラスヘッダ
 *
 * @author  Taro.Yamada
 *
 * @date    2019/09/17
 */

#ifndef BBSMENUWEBLOADER_H
#define BBSMENUWEBLOADER_H

#include <QObject>
#include <QStandardItemModel>

#include "bbsmenuloaderbase.h"

class QNetworkAccessManager;

namespace bbsmenu {

/**
 * @brief The BbsMenuWebLoader class
 */
class BbsMenuWebLoader : public BbsMenuLoaderBase {
    Q_OBJECT
public:
    BbsMenuWebLoader(QNetworkAccessManager * const pManager, QObject *parent = nullptr);
    QStandardItemModel *loadBbsMenu();

signals:

public slots:

private:
    QNetworkAccessManager *_pManager;

    QStringList loadBbsMenuFromWeb();
    QStandardItemModel* parseBbsMenu(const QStringList clstMenuHtml);
    int buildMenuModel(const int startLineIndex, const QStringList menuHtml, QStandardItem * const pCategoryItem);
    bool extractTitleAndUrlFromHtml(const QString strLineText, QString *title, QString *url);
    QString categoryName(const QString strLineText);
};

}   // end of namespace

#endif // BBSMENUWEBLOADER_H
