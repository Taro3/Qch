/**
 *
 * @file    bbsmenu.h
 *
 * @brief   板一覧(メニュー一覧)管理クラスヘッダ
 *
 * @author  Taro.Yamada
 *
 * @date    2019/01/30
 *
 */

#ifndef BBSMENU_H
#define BBSMENU_H

#include <QObject>

class QStandardItemModel;
class QNetworkAccessManager;

namespace bbsmenu {

/**
 * @enum    The LOAD_FROM enum
 * 			BBSメニュー読込先定義
 */
enum class LOAD_FROM : int {
    FROM_CACHE,	//!< キャッシュから読込
    FROM_WEB,	//!< Webから読込
};

/**
 * @brief   The BbsMenu class
 *          板一覧データクラス
 */
class BbsMenu : public QObject {
    Q_OBJECT

public:
    BbsMenu(QNetworkAccessManager * const pManager, QObject * const parent = nullptr);
    QStandardItemModel* menuModel(const LOAD_FROM from = LOAD_FROM::FROM_CACHE);
    void saveBbsMenu(const QStandardItemModel * const pModel);

signals:
    void beginLoadBbsMenu();    //!< BBSメニュー読込開始シグナル
    void endLoadBbsMenu();      //!< BBSメニュー読込完了シグナル

public slots:

private:
    QNetworkAccessManager *_pcNetworkAccessManager; //!< ネットワークアクセスマネージャインスタンスポインタ
};

}   // end of namespace

#endif // BBSMENU_H
