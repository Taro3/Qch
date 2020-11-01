/**
 * @file    mainwindow.cpp
 *
 * @brief   メインウィンドウクラス実装
 *
 * @author  Taro.Yamada
 *
 * @date    2019/01/31
 */

#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItemModel>
#include <QApplication>
#include <QXmlStreamWriter>
#include <QFile>
#include <QMouseEvent>
#include <QToolTip>
#include <QFont>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QImage>
#include <QBuffer>
#include <QRegularExpression>
#include <QException>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hbbsutil.h"
#include "writemessagedialog/writemessagedialog.h"
#include "bbsmenu/bbsmenu.h"
#include "threadlist/threadlist.h"

using namespace bbsmenu;
using namespace messagedlg;
using namespace threadlist;

/**
 * @brief       MainWindow::MainWindow
 *              メインウィンドウコンストラクタ
 * @param[in]   parent  親ウィジェットポインタ
 */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), m_pcMenuModel(nullptr)
  , m_pcThreadListModel(nullptr), m_clstPostData(QList<POST_DATA>()), m_strThreadUrl(""), m_strHostName("")
  , m_strBbsName(""), m_strThreadId(""), m_pcBbsMenu(nullptr), m_pcNetwoekAccessManager(nullptr)
{
    ui->setupUi(this);

    m_pcNetwoekAccessManager = new QNetworkAccessManager(this);

    ui->action_Write_Message->setEnabled(false);    // メッセージ書込無効化

    // スレッドビュー初期化
    initThreadView();

    // メニューの初期化(板一覧初期化)
    m_pcBbsMenu = new BbsMenu(m_pcNetwoekAccessManager, this);
    m_pcMenuModel = m_pcBbsMenu->menuModel(LOAD_FROM::FROM_CACHE);
    if (!m_pcMenuModel->rowCount())
        qDebug() << "Category count is zero.";
    ui->treeViewCategory->setModel(m_pcMenuModel);
}

/**
 * @brief   MainWindow::initThreadView
 *          スレッドビューの初期化
 */
void MainWindow::initThreadView() {
    auto pTextEditChildWidget = getThreadChildWidget();
    pTextEditChildWidget->installEventFilter(this);     // メッセージ表示部のイベントフィルタ設定
    pTextEditChildWidget->setMouseTracking(true);       // マウストラッキングON
    pTextEditChildWidget->setCursor(Qt::ArrowCursor);   // ビュー上でのカーソルを矢印に変更
}

/**
 * @brief   MainWindow::~MainWindow
 *          メインウィンドウデストラクタ
 */
MainWindow::~MainWindow() {
    delete ui;
}

/**
 * @brief   MainWindow::initSpliiterSize
 *          初期の板一覧幅とスレ一覧の高さ計算＆設定
 */
void MainWindow::initSpliiterSize() {
    QList<int> lstSizes;
    lstSizes << int(this->width() * .3f) << int(this->width() * .7f);
    ui->splitter_2->setSizes(lstSizes);
    lstSizes.clear();
    lstSizes << int(this->height() * .2f) << int(this->height() * .8f);
    ui->splitter->setSizes(lstSizes);
}

/**
 * @brief   MainWindow::eventFilter
 *          イベントフィルタ定義
 * @param   watched[in]   オブジェクトポインタ
 * @param   event[in]     イベント依存オブジェクト
 * @return  処理結果
 * @retval  true    処理済
 * @retval  false   未処理
 */
bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    // スレッドビューウィジェットのイベント
    auto pTextEditChildWidget = getThreadChildWidget();
    if (watched == pTextEditChildWidget) {
        // マウス移動イベント
        if (event->type() == QEvent::MouseMove) {
            // マウス位置のリンクURLを取得
            auto pMouseMoveEvent = static_cast<QMouseEvent*>(event);
            auto strAnchorUrl = ui->textEditThread->anchorAt(pMouseMoveEvent->pos());
            if (!strAnchorUrl.isEmpty()) {
                static const auto STR_NUMBER_RANGE_ANCHOR_RE = "(\\d+)-(\\d+)";
                QString strTooTipText;
                auto isNumberAnchor = false;
                auto nAnchorNumber = strAnchorUrl.toInt(&isNumberAnchor);
                if (isNumberAnchor) {
                    if (nAnchorNumber <= m_clstPostData.count()) {
                        auto strTempPostedHtml = m_clstPostData[nAnchorNumber - 1].strHtml;  // 最後の<br>を削除
                        removeLastBr(strTempPostedHtml);
                        strTooTipText = strTempPostedHtml;
                    }
                } else {
                    QRegularExpression reNumberRangeAnchor{STR_NUMBER_RANGE_ANCHOR_RE
                                                           , QRegularExpression::CaseInsensitiveOption};
                    auto reMatchNumberRangeAnchor = reNumberRangeAnchor.match(strAnchorUrl);
                    if (reMatchNumberRangeAnchor.hasMatch()) {
                        auto nAnchorStartNumber = reMatchNumberRangeAnchor.captured(1).toInt();
                        auto nAnchorEndNumber = reMatchNumberRangeAnchor.captured(2).toInt();
                        if (nAnchorStartNumber <= m_clstPostData.count()
                                && nAnchorEndNumber <= m_clstPostData.count())
                        {
                            if (nAnchorStartNumber > nAnchorEndNumber)
                                qSwap(nAnchorStartNumber, nAnchorEndNumber);    //std::swap(nAnchorStartNumber, nAnchorEndNumber);
                            for (int i = nAnchorStartNumber; i <= nAnchorEndNumber; ++i) {
                                auto strTempPostedHtml = m_clstPostData[i - 1].strHtml;  // レス間の改行幅が大きすぎるので最後の<br>は削除する
                                removeLastBr(strTempPostedHtml);
                                strTooTipText += strTempPostedHtml;
                            }
                        }
                    } else {
                        if (checkImageAnchor(strAnchorUrl)) {
                            auto cPreviousCursor = this->cursor();
                            setCursors(Qt::WaitCursor);
//                            qobject_cast<QWidget*>(ui->textEditThread->children().at(0))->setCursor(Qt::WaitCursor);
                            auto cImage = fetchImage(strAnchorUrl);
                            if (!cImage.isNull()) {
                                cImage = cImage.scaledToWidth(128);
                                QFileInfo cFileInfo{strAnchorUrl};
                                QByteArray baData;
                                QBuffer cBuffer{&baData};
                                cBuffer.open(QIODevice::WriteOnly);
                                cImage.save(&cBuffer, cFileInfo.suffix().toUpper().toLatin1().data());
                                strTooTipText = QString("<img src=\"data:image/%0;base64, %1\">")
                                        .arg(cFileInfo.suffix()).arg(QString(baData.toBase64()));
                            } else {
                                strTooTipText = strAnchorUrl;   // 上記のどのアンカでもない場合はとりあえずURL表示する
                            }
                            setCursors(cPreviousCursor);
//                            qobject_cast<QWidget*>(ui->textEditThread->children().at(0))->setCursor(cPreviousCursor);
                        } else {
                            strTooTipText = strAnchorUrl;   // 上記のどのアンカでもない場合はとりあえずURL表示する
                        }
                    }
                }
                auto ptHtmlMousePos = ui->textEditThread->mapToGlobal(pMouseMoveEvent->pos());
                QToolTip::showText(ptHtmlMousePos, strTooTipText);
            } else {
                QToolTip::showText(QPoint(), "");
            }
//            return true;
        }
    }

    return false;
}

/**
 * @brief       MainWindow::closeEvent
 *              メインウィンドウのクローズイベントハンドラ
 * @param[in]   event   イベントデータ
 * @return      なし
 */
void MainWindow::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)

    // カテゴリリスト(板一覧)データ保存
    m_pcBbsMenu->saveBbsMenu(qobject_cast<QStandardItemModel*>(ui->treeViewCategory->model()));
}

/**
 * @brief   MainWindow::on_actionUpdate_Category_List_triggered
 *          メニュー更新
 * @return  なし
 */
void MainWindow::on_actionUpdate_Category_List_triggered() {
//    HBBSUtil util;

    auto prevCursor = this->cursor();
    setCursors(Qt::WaitCursor);

    // 既存のメニューデータ削除
    if (m_pcMenuModel) {
        delete m_pcMenuModel;
        m_pcMenuModel = nullptr;
    }
    // メニューをHTMLから取得
    m_pcMenuModel = m_pcBbsMenu->menuModel(LOAD_FROM::FROM_WEB);
    if (m_pcMenuModel->rowCount() == 0)
        qDebug() << "bbsmenu row count is zero.";
    else
        ui->treeViewCategory->setModel(m_pcMenuModel);
//    m_pcMenuModel = util.menuModel();
//    if (m_pcMenuModel->rowCount() == 0) {
//        qDebug() << "bbsmenu couldn't load.";
//        delete m_pcMenuModel;
//        m_pcMenuModel = nullptr;
//    } else {
//        ui->treeViewCategory->setModel(m_pcMenuModel);
//    }

    setCursors(prevCursor);
}

/**
 * @brief MainWindow::on_treeViewCategory_doubleClicked
 * @param index
 */
void MainWindow::on_treeViewCategory_doubleClicked(const QModelIndex &index) {
    const auto pModel = qobject_cast<const QStandardItemModel*>(index.model());
    if (pModel && index.parent() == pModel->invisibleRootItem()->index())   // トップ階層のアイテムの場合は何もしない
        return;

    auto strUrl = index.data(Qt::UserRole + 1).toString();
    if (strUrl.isEmpty()) {
        qDebug() << __FUNCTION__ << " " << __LINE__ << " URL is empty.";
        return;
    }

    auto cPrevCursor = this->cursor();
    setCursors(Qt::WaitCursor);

    //HBBSUtil util;
    auto pPrevModel = qobject_cast<QStandardItemModel*>(ui->listViewThreads->model());
    //m_pcThreadListModel = util.threadListModel(strUrl);
    ThreadList tl(m_pcNetwoekAccessManager);
    m_pcThreadListModel = tl.threadListModel(strUrl);
    if (m_pcThreadListModel->rowCount() == 0) {
        qDebug() << __FUNCTION__ << " " << __LINE__ << " thread list empty.";
    } else {
        ui->listViewThreads->setModel(m_pcThreadListModel);
        delete pPrevModel;
    }

    setCursors(cPrevCursor);
}

/**
 * @brief MainWindow::on_listViewThreads_doubleClicked
 * @param index
 */
void MainWindow::on_listViewThreads_doubleClicked(const QModelIndex &index) {
    auto cPrevCursor = this->cursor();
    setCursors(Qt::WaitCursor);

    HBBSUtil util;
    ui->textEditThread->clear();
    auto sData = index.data(Qt::UserRole + 1).value<THREAD_TITLE_DATA>();
    updateThreadView(sData.strThreadUrl);
    auto cCursor = ui->textEditThread->textCursor();
    cCursor.movePosition(QTextCursor::Start);
    ui->textEditThread->setTextCursor(cCursor);

    setCursors(cPrevCursor);
}

/**
 * @brief MainWindow::removeLastBr
 * @param strHtml
 * @return
 */
bool MainWindow::removeLastBr(QString &strHtml) {
    auto isReplaced = false;
    auto nDeleteIndex = -1;
    if ((nDeleteIndex = strHtml.lastIndexOf("<br>", -1, Qt::CaseInsensitive)) >= 0) {
        strHtml.remove(nDeleteIndex, 4);
        isReplaced = true;
    }

    return isReplaced;
}

/**
 * @brief MainWindow::checkImageAnchor
 * @param strHtml
 * @return
 */
bool MainWindow::checkImageAnchor(const QString strHtml) {
    static const QStringList STR_IMAGE_SUFFIXES = {"jpg", "jpeg", "gif", "png", "bmp", "pbm", "pgm", "ppm", "xbm"
                                                   , "xpm"};
    QFileInfo cImageInfo{strHtml};
    auto strImageSuffix = cImageInfo.suffix();
    auto isImageUrl = false;
    for (auto it = STR_IMAGE_SUFFIXES.constBegin(); it != STR_IMAGE_SUFFIXES.constEnd(); ++it) {
        if (strImageSuffix.compare(*it, Qt::CaseInsensitive) == 0) {
            isImageUrl = true;
            break;
        }
    }

    return isImageUrl;
}

/**
 * @brief MainWindow::fetchImage
 * @param strImageUrl
 * @return
 */
QImage MainWindow::fetchImage(const QString strImageUrl) {
    QImage cImage;
    QNetworkAccessManager cManager;
    auto pReply = cManager.get(QNetworkRequest(QUrl(strImageUrl)));
    while (pReply->isRunning())
        qApp->processEvents();
    if (pReply->error() != QNetworkReply::NoError) {
        qDebug() << pReply->errorString();
        return cImage;
    }
    QFileInfo cFileInfo{strImageUrl};
    cImage.load(pReply, cFileInfo.suffix().toUpper().toLatin1().data());

    return cImage;
}

/**
 * @brief MainWindow::updateThreadView
 * @param strThreadUrl
 * @return
 */
bool MainWindow::updateThreadView(const QString strThreadUrl) {
    HBBSUtil util;
    ui->textEditThread->clear();
    auto strThreadHtml = util.loadThreadHtml(strThreadUrl);
    m_clstPostData.clear();
    if (!util.postData(strThreadHtml, m_clstPostData) || m_clstPostData.count() == 0) {
        qDebug() << "post data failed.";
        qDebug() << strThreadHtml;
        ui->textEditThread->setHtml(strThreadHtml);
        return false;
    }
    QString strHtml;
    for (auto i = 0; i < m_clstPostData.count(); ++i) {
        strHtml += m_clstPostData[i].strHtml;
//        QString strPosted = m_clstPostData[i].strHtml;
//        ui->textEditThread->append(strPosted);
//        ui->textEditThread->append(strPosted);
    }
QRegularExpression r{">(.*?)<"};
auto n = 0;
while (true) {
    auto m = r.match(strHtml, n);
    if (!m.hasMatch())
        break;
    auto s = m.captured(1);
    s.replace(' ', "&nbsp;");
    s.replace("　", "&nbsp;&nbsp;");
//    s.replace("　", "&#12288;");
    strHtml.replace(m.capturedStart(1), m.capturedLength(1), s);
    n = m.capturedStart() + m.capturedLength() + 1;
}
    ui->textEditThread->append(strHtml);

    QRegularExpression reThreadUrl{"^http://.*?\\.5ch\\.net/test/read\\.cgi/(.*?)/(.*?)$"
                                   , QRegularExpression::CaseInsensitiveOption};
    auto reMatchThreadUrl = reThreadUrl.match(strThreadUrl);
    if (!reMatchThreadUrl.hasMatch()) {
        qDebug() << "can't find bbs and thread name.";
        m_strThreadUrl.clear();
        m_strHostName.clear();
        m_strBbsName.clear();
        m_strThreadId.clear();
    } else {
        m_strThreadUrl = strThreadUrl;
        m_strHostName  = QUrl(strThreadUrl).host();
        m_strBbsName   = reMatchThreadUrl.captured(1);
        m_strThreadId  = reMatchThreadUrl.captured(2);
    }

    ui->action_Write_Message->setEnabled(true);

    return true;
}

/**
 * @brief MainWindow::setCursors
 * @param cCursor
 */
void MainWindow::setCursors(const QCursor cCursor) {
    this->setCursor(cCursor);
    auto pTextEditChildWidget = qobject_cast<QWidget*>(ui->textEditThread->children().at(0));
    pTextEditChildWidget->setCursor(cCursor);
}

/**
 * @brief MainWindow::on_action_Write_Message_triggered
 */
void MainWindow::on_action_Write_Message_triggered() {
    WriteMessageDialog cDialogMessage{this};
    cDialogMessage.setServerInfo(m_strHostName, m_strBbsName, m_strThreadId);
    cDialogMessage.setModal(true);
    if (cDialogMessage.exec() == QDialog::Accepted) {
        updateThreadView(m_strThreadUrl);
        auto cCursor = ui->textEditThread->textCursor();
        cCursor.movePosition(QTextCursor::End);
        cCursor.setPosition(ui->textEditThread->document()->toPlainText().length());
        ui->textEditThread->setTextCursor(cCursor);
    }
}
