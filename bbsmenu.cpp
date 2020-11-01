#include <QStandardItemModel>
#include <QRegularExpression>
#include <QDebug>
#include <QApplication>
#include <QFile>
#include <QXmlStreamWriter>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextCodec>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QException>

#include "bbsmenu.h"
#include "settings.h"
#include "qchexception.h"

/***********************************************************************************************************************
 *  グローバル関数
 **********************************************************************************************************************/

//======================================================================================================================
/**
 * @brief       BbsManu::BbsManu
 * @details     コンストラクタ
 * @param[in]   parent  親オブジェクトポインタ
 * @return      なし
 */
BbsMenu::BbsMenu(QNetworkAccessManager * const pManager, QObject *parent)
    : QObject(parent)
    , m_clstIgnoreCategories(QStringList())
    , m_clstIgnoreThreads(QStringList())
    , m_pcNetworkAccessManager(nullptr)
{
    Settings cSettings;

    m_clstIgnoreCategories = cSettings.ignoreCategories();  // 無視カテゴリリスト初期化
    m_clstIgnoreThreads = cSettings.ignoreThreads();        // 無視スレッドリスト初期化

    // ネットワークアクセスマネージャポインタを退避する
    if (!pManager) {
        qDebug() << "Fatal! " << __FILE__ << __FUNCTION__ << __LINE__ << ": pManager is nullptr.";
    }
    Q_ASSERT(pManager);
    m_pcNetworkAccessManager = pManager;
}

//======================================================================================================================
/**
 * @brief 		BbsMenu::menuModel
 *              メニュー用モデル生成
 * @param[in]	from	メニュー読込先指定(FROM_CACHE:キャッシュ FROM_WEB:ウェブ)
 * @return      メニュー用モデルデータポインタ
 */
QStandardItemModel *BbsMenu::menuModel(const LOAD_FROM from)
{
    // すでに一回ネットからDLしているメニューモデルを返す
    if (from == LOAD_FROM::FROM_CACHE) {
        auto pcMenuModel = loadBbsMenu();
        if (pcMenuModel) {
            return pcMenuModel;
        }
    }
    // ネットからメニューのHTMLを読み込む
    auto clstMenuHtml = loadBbsMenuFromWeb();

    // メニューデータを解析する
    return ParseBbsMenu(clstMenuHtml);
}

//======================================================================================================================
/**
 * @brief       BbsManu::saveBbsMenu
 *              カテゴリリスト保存
 * @param[in]   pModel  保存するカテゴリモデル
 * @return      処理結果
 * @retval      true    正常終了
 * @retval      false   エラー
 */
bool BbsMenu::saveBbsMenu(const QStandardItemModel * const pModel)
{
    Q_ASSERT(pModel);

    auto strFilename = bbsmenuFilePathName();
    if (!QFile::exists(strFilename)) {
        QFileInfo cFileInfo{strFilename};
        auto cDir = cFileInfo.absoluteDir();
        if (!cDir.exists()) {
            if (!cDir.mkpath(cDir.path())) {
                qDebug() << "Can't make directory.";
                return false;
            }
        }
    }
    QFile cFile{strFilename};

    if (!cFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << strFilename << " can't create.";
        return false;
    }

    QXmlStreamWriter xml{&cFile};
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("categories");
    xml.writeAttribute("count", QString::number(pModel->rowCount()));

    for (auto i = 0; i < pModel->rowCount(); ++i) {
        // カテゴリ名保存
        xml.writeStartElement("category");
        auto pItem = pModel->item(i);
        auto strCategoryName = pItem->text();
        xml.writeAttribute("name", strCategoryName);
        xml.writeAttribute("count", QString::number(pItem->rowCount()));

        // スレッド一覧保存
        for (auto j = 0; j < pItem->rowCount(); ++j) {
            auto pThreadItem = pItem->child(j);
            auto strTitle     = pThreadItem->text();
            auto strUrl       = pThreadItem->data().toString();
            xml.writeTextElement("title", strTitle);
            xml.writeTextElement("url", strUrl);
        }

        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndDocument();
    cFile.flush();
    cFile.close();

    return true;
}

/***********************************************************************************************************************
 *  プライベート関数
 **********************************************************************************************************************/

//----------------------------------------------------------------------------------------------------------------------
/**
 * @brief 		BbsMenu::loadBbsMenu
 *              カテゴリリスト読込
 * @return 		生成されたカテゴリ用モデル
 */
QStandardItemModel* BbsMenu::loadBbsMenu()
{
    auto strFilename = bbsmenuFilePathName();
    QFile cFile(strFilename);

    if (!cFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << strFilename << " can't open.";
        return nullptr;
    }

    QXmlStreamReader xml{&cFile};
    if (xml.readNext() != QXmlStreamReader::StartDocument) {
        qDebug() << "invalid bbsmenu data.";
        return nullptr;
    }

    xml.readNextStartElement();
    auto nCategoryCount = xml.attributes().value("count").toInt();

    auto pModel = new QStandardItemModel;

    for (auto i = 0; i < nCategoryCount; ++i) {
        xml.readNextStartElement();
        auto strCategoryName = xml.attributes().value("name").toString();    // カテゴリ名
        auto pItem = new QStandardItem{strCategoryName};
        auto nItemCount = xml.attributes().value("count").toInt();           // スレッド数

        for (auto j = 0; j < nItemCount; ++j) {
            xml.readNextStartElement();
            auto strTitle = xml.readElementText();                   // スレッドタイトル
            xml.readNextStartElement();
            auto strUrl = xml.readElementText();                     // スレッドURL
            auto pThreadItem = new QStandardItem{strTitle};
            pThreadItem->setData(strUrl);
            pItem->appendRow(pThreadItem);
        }

        xml.readNextStartElement();
        pModel->appendRow(pItem);
    }

    cFile.close();

    return pModel;
}

//----------------------------------------------------------------------------------------------------------------------
/**
 * @brief BbsMenu::BuildMenuModel
 *        カテゴリアイテムの子アイテムとしてスレッド名＋URLのアイテムを追加する
 * @param[in]       startLineIndex    HTML内のスレッド文字列検索開始行インデックス
 * @param[in]       menuHtml          カテゴリ内のスレッド一覧を含むHTML
 * @param[in out]   pCategoryItem     スレッド一覧を追加するカテゴリアイテム
 * @return 次カテゴリの検索を開始するHTMLの行インデックス番号
 */
int BbsMenu::BuildMenuModel(const int startLineIndex, const QStringList menuHtml, QStandardItem* const pCategoryItem)
{
    // スレッドリンク行処理
    auto nHtmlThreadLineIndex = startLineIndex;
    forever {
        auto strThreadLine = menuHtml[nHtmlThreadLineIndex];
        QString strThreadName;
        QString strThreadUrl;

        // マッチなし?
        if (!extractTitleAndUrlFromHtml(strThreadLine, strThreadName, strThreadUrl)) {
            break;
        }

        // コメント行または無視スレッド
        if (strThreadName.isEmpty() && strThreadUrl.isEmpty()) {
            continue;
        }

        auto pThreadItem = new QStandardItem(strThreadName);    // 子アイテム生成
        pThreadItem->setData(strThreadUrl);                     // data(USER_ROLE + 1) に URL をセット
        pCategoryItem->appendRow(pThreadItem);                  // カテゴリの子として追加する

        ++nHtmlThreadLineIndex;
    }

    return nHtmlThreadLineIndex;
}

//----------------------------------------------------------------------------------------------------------------------
/**
 * @brief   BbsManu::loadBbsMenuFromWeb
 *          ホストからメニューのHTMLを取得する
 * @return  メニューHTMLデータ
 *          エラーの場合は空のQStringListを返す
 */
QStringList BbsMenu::loadBbsMenuFromWeb()
{
    QStringList clstMenu;
    Settings cSetting;

    auto strUrl = cSetting.value(Settings::eSETTING_NAME::SETTING_BBSMENU_URL);
    auto pcReply = m_pcNetworkAccessManager->get(QNetworkRequest(QUrl(strUrl)));
    while (pcReply->isRunning()) {
        qApp->processEvents();
    }

    if (pcReply->error() != QNetworkReply::NetworkError::NoError) {
        qDebug() << pcReply->errorString();
        pcReply->close();
        return clstMenu;
    }

    auto baMenuHtml  = pcReply->readAll();
    auto pHtmlCodec = QTextCodec::codecForHtml(baMenuHtml);
    auto strMenuHtml = pHtmlCodec->toUnicode(baMenuHtml);
    clstMenu = strMenuHtml.split('\n');

    return clstMenu;
}

//----------------------------------------------------------------------------------------------------------------------
/**
 * @brief   BbsManu::bbsmenuFilePathName
 *          bbsmenuファイルのパスとファイル名取得
 * @return  bbsmenuファイルのパスとファイル名
 */
QString BbsMenu::bbsmenuFilePathName()
{
    Settings cSettings;
    auto strFilename = cSettings.value(Settings::SETTING_DATA_FILE_PATH) + "/"
            + cSettings.value(Settings::SETTING_BBSMENU_FILE_NAME);

    return strFilename;
}

//----------------------------------------------------------------------------------------------------------------------
/**
 * @brief       BbsManu::categoryName
 *              メニューのHTML1行からカテゴリ名を抽出する
 * @param[in]   strLineText HTML行データ
 * @return      カテゴリ名
 *              エラーの場合は空文字列を返す
 */
QString BbsMenu::categoryName(const QString strLineText)
{
    static const QRegularExpression reCategoryTitle{"<B>(.*?)</B>", QRegularExpression::CaseInsensitiveOption};

    auto reMatchCategory = reCategoryTitle.match(strLineText);
    if (!reMatchCategory.hasMatch()) {
        return "";
    }
    auto strCategoryName = reMatchCategory.captured(1).trimmed();

    // 無視カテゴリリストチェック
    if (m_clstIgnoreCategories.indexOf(strCategoryName) >= 0) {
        return "";
    }

    return strCategoryName;
}

//----------------------------------------------------------------------------------------------------------------------
/**
 * @brief BbsMenu::extractTitleAndUrlFromHtml
 *        メニューのHTMLからスレッドタイトルとURLを抽出する
 * @param[in] strLineText   HTMLの行データ
 * @param[out] title    スレッドタイトル文字列
 * @param[out] url      スレッドURL文字列
 * @return 処理結果
 * @retval  true    一致あり
 * @retval  false   一致なし
 * @note    コメント行または無視スレッドの場合はtitleとurl共に空文字列を設定する
 */
bool BbsMenu::extractTitleAndUrlFromHtml(const QString strLineText, QString &title, QString &url)
{
    static const auto THREAD_LINE_PATTERN = "^<A\\sHREF.*?=.*?(http://.*?)>(.*?)</A>.*";
    static const QRegularExpression reThreadLine{THREAD_LINE_PATTERN, QRegularExpression::CaseInsensitiveOption};
    static const QRegularExpression reCommentLine{"^<!--.*?-->", QRegularExpression::CaseInsensitiveOption};

    // コメント?
    if (strLineText.indexOf(reCommentLine) >= 0) {
        return true;
    }

    // マッチなし?
    auto reMatchThreadLine = reThreadLine.match(strLineText);
    if (!reMatchThreadLine.hasMatch()) {
        return false;
    }

    auto tmpTitle = reMatchThreadLine.captured(2).trimmed();    // タイトル
    auto tmpUrl = reMatchThreadLine.captured(1).trimmed();      // URL

    // 無視スレッド名?
    if (m_clstIgnoreThreads.indexOf(tmpTitle) >= 0) {
        return true;
    }

    title = tmpTitle;
    url = tmpUrl;

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
/**
 * @brief		BbsMenu::ParseBbsMenu
 * 				メニューのHTMLを解析しデータに分割する
 * @param[in]	clstMenuHtml	メニューのHTML
 */
QStandardItemModel* BbsMenu::ParseBbsMenu(const QStringList clstMenuHtml)
{
    static const auto CATEGORY_PATTERN = "^<br><br><B>.*?</B><br>.*";
    static const QRegularExpression reCategory(CATEGORY_PATTERN, QRegularExpression::CaseInsensitiveOption);

    auto pcMenuModel = new QStandardItemModel;
    auto nHtmlCategoryLineIndex = -1;
    while ((nHtmlCategoryLineIndex = clstMenuHtml.indexOf(reCategory, nHtmlCategoryLineIndex + 1)) >= 0) {
        auto strLine = clstMenuHtml[nHtmlCategoryLineIndex];

        // カテゴリ名文字列取得
        auto strCategoryName = categoryName(strLine);
        if (strCategoryName.isEmpty()) {
            continue;
        }

        auto pCategoryItem = new QStandardItem(strCategoryName);
        // カテゴリアイテムの子としてスレッド名とURLを追加する
        nHtmlCategoryLineIndex = BuildMenuModel(nHtmlCategoryLineIndex, clstMenuHtml, pCategoryItem);
        pcMenuModel->appendRow(pCategoryItem);
    }

    return pcMenuModel;
}
