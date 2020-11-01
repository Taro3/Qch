#ifndef BBSMENU_H
#define BBSMENU_H

#include <QObject>

class QStandardItemModel;
class QNetworkAccessManager;
class QStandardItem;

//**********************************************************************************************************************
/**
 * @brief The BbsMenu class
 */
class BbsMenu : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 	The LOAD_FROM enum
     * 			BBSメニュー読込先定義
     */
    enum LOAD_FROM {
        FROM_CACHE,	//!< キャッシュから読込
        FROM_WEB,	//!< Webから読込
    };

    explicit            BbsMenu(QNetworkAccessManager * const pManager, QObject *parent = nullptr);
    QStandardItemModel* menuModel(const LOAD_FROM from);
    bool                saveBbsMenu(const QStandardItemModel * const pModel);

signals:

public slots:

private:
    QStringList m_clstIgnoreCategories;                 //!< 無視カテゴリリスト
    QStringList m_clstIgnoreThreads;                    //!< 無視スレッドリスト
    QNetworkAccessManager *m_pcNetworkAccessManager;    //!< ネットワークアクセスマネージャインスタンスポインタ

    QStandardItemModel *loadBbsMenu();
    QStringList loadBbsMenuFromWeb();
    QString bbsmenuFilePathName();
    QString categoryName(const QString strLineText);
    bool extractTitleAndUrlFromHtml(const QString strLineText, QString &title, QString &url);
    int BuildMenuModel(const int startLineIndex, const QStringList menuHtml, QStandardItem * const pCategoryItem);
    void parseMenu();
    QStandardItemModel* ParseBbsMenu(const QStringList clstMenuHtml);
};

#endif // BBSMENU_H
