#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_mainwindow.h"
#include "hbbsutil.h"

class QStandardItemModel;
namespace bbsmenu {
class BbsMenu;
}
class QNetworkAccessManager;

namespace Ui {
class MainWindow;
}

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void initSpliiterSize();
    void initThreadView();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void on_actionUpdate_Category_List_triggered();
    void on_treeViewCategory_doubleClicked(const QModelIndex &index);
    void on_listViewThreads_doubleClicked(const QModelIndex &index);
    void on_action_Write_Message_triggered();

private:
    Ui::MainWindow  *ui;
    QStandardItemModel  *m_pcMenuModel;
    QStandardItemModel  *m_pcThreadListModel;
    QList<POST_DATA>    m_clstPostData;
    QString m_strThreadUrl;
    QString m_strHostName;
    QString m_strBbsName;
    QString m_strThreadId;
    bbsmenu::BbsMenu    *m_pcBbsMenu;
    QNetworkAccessManager   *m_pcNetwoekAccessManager;

    inline QWidget* getThreadChildWidget();
    bool    removeLastBr(QString &strHtml);
    bool    checkImageAnchor(const QString strHtml);
    QImage  fetchImage(const QString strImageUrl);
    bool    updateThreadView(const QString strThreadUrl);
    void    setCursors(const QCursor cCursor);
};

//----------------------------------------------------------------------------------------------------------------------
/**
 * @brief   MainWindow::getThreadChildWidget
 *          スレッドウィジェットの子ウィジェットを取得する
 * @return  子ウィジェットポインタ
 */
QWidget *MainWindow::getThreadChildWidget() {
    return qobject_cast<QWidget*>(ui->textEditThread->children().at(0));   // スレ表示ウィジェットの最初の子を取得
}

#endif // MAINWINDOW_H
