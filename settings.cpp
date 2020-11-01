#include <QSettings>
#include <QDebug>
#include <QStandardPaths>
#include <QApplication>

#include "settings.h"

QString Settings::m_strConfigFilePathName = "";

/**
 *  @brief  SETTING_NAMES
 *          設定項目列挙子
 */
const QString Settings::SETTING_NAMES[] = {
    "DATA_FILE_PATH",       //!< bbsmenu.xmlのパスとファイル名
    "BBSMENU_FILE_NAME",    //!< bbsmenuファイル名
    "BBSMENU_URL",          //!< bbsmenuのURL
    "VIEW_FONT_NAME",       //!< ログの表示フォント
    "FIRST_POST",           //!< 最初の書込状態
};

/**
 * @brief Settings::Settings
 * @param parent
 */
Settings::Settings(QObject *parent) : QObject{parent} , m_pcSettings{nullptr} {
    if (m_strConfigFilePathName.isEmpty()) {
        auto clstConfigPaths = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
        if (clstConfigPaths.count() >= 0)
            m_strConfigFilePathName = clstConfigPaths[0];
        else
            m_strConfigFilePathName = qApp->applicationDirPath();
        m_strConfigFilePathName += "/Qch.conf";
    }

    m_pcSettings = new QSettings{m_strConfigFilePathName, QSettings::NativeFormat, this};
}

/**
 * @brief Settings::~Settings
 */
Settings::~Settings() {
    delete m_pcSettings;
    m_pcSettings = nullptr;
}

/**
 * @brief Settings::value
 * @param eSettingName
 * @return
 */
QString Settings::value(const Settings::eSETTING_NAME eSettingId) {
    static const auto STR_DEFAULT_BBSMENU_FILE_NAME = "bbsmenu.xml";
    static const auto STR_DEFAULT_BBSMENU_URL = "http://menu.5ch.net/bbsmenu.html";
    static const auto STR_DEFAULT_VIEW_FONT_NAME = "IPAモナー Pゴシック";
    static const auto STR_DEFAULT_FIRST_POST = "yes";
    QString strSettingValue;

    switch (eSettingId) {
    case eSETTING_NAME::SETTING_DATA_FILE_PATH:
        strSettingValue = m_pcSettings->value(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_DATA_FILE_PATH)]).toString();
        if (strSettingValue.isEmpty()) {
            strSettingValue = makeDataFilePath();
            m_pcSettings->setValue(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_DATA_FILE_PATH)], strSettingValue);
        }
        return strSettingValue;

    case eSETTING_NAME::SETTING_BBSMENU_FILE_NAME:
        strSettingValue =
                m_pcSettings->value(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_BBSMENU_FILE_NAME)]).toString();
        if (strSettingValue.isEmpty()) {
            strSettingValue = STR_DEFAULT_BBSMENU_FILE_NAME;
            m_pcSettings->setValue(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_BBSMENU_FILE_NAME)], strSettingValue);
        }
        return strSettingValue;

    case eSETTING_NAME::SETTING_BBSMENU_URL:
        strSettingValue = m_pcSettings->value(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_BBSMENU_URL)]).toString();
        if (strSettingValue.isEmpty()) {
            strSettingValue = STR_DEFAULT_BBSMENU_URL;
            m_pcSettings->setValue(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_BBSMENU_URL)], strSettingValue);
        }
        return strSettingValue;

    case eSETTING_NAME::SETTING_VIEW_FONT_NAME:
        strSettingValue = m_pcSettings->value(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_VIEW_FONT_NAME)]).toString();
        if (strSettingValue.isEmpty()) {
            strSettingValue = STR_DEFAULT_VIEW_FONT_NAME;
            m_pcSettings->setValue(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_VIEW_FONT_NAME)], strSettingValue);
        }
        return strSettingValue;

    case eSETTING_NAME::SETTING_FIRST_POST:
        strSettingValue = m_pcSettings->value(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_FIRST_POST)]).toString();
        if (strSettingValue.isEmpty()) {
            strSettingValue = STR_DEFAULT_FIRST_POST;
            m_pcSettings->setValue(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_FIRST_POST)], strSettingValue);
        }
        return strSettingValue;
    }

    qDebug() << QString{__FILE__} + " " + QString{__LINE__} + ": Unknown setting name.";

    return "";
}

/**
 * @brief Settings::setValue
 * @param eSettingId
 * @param strValue
 */
void Settings::setValue(const Settings::eSETTING_NAME eSettingId, const QString strValue) {
    switch (eSettingId) {
    case eSETTING_NAME::SETTING_DATA_FILE_PATH:
        m_pcSettings->setValue(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_DATA_FILE_PATH)], strValue);
        return;

    case eSETTING_NAME::SETTING_BBSMENU_FILE_NAME:
        m_pcSettings->setValue(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_BBSMENU_FILE_NAME)], strValue);
        return;

    case eSETTING_NAME::SETTING_BBSMENU_URL:
        m_pcSettings->setValue(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_BBSMENU_URL)], strValue);
        return;

    case eSETTING_NAME::SETTING_VIEW_FONT_NAME:
        m_pcSettings->setValue(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_VIEW_FONT_NAME)], strValue);
        return;

    case eSETTING_NAME::SETTING_FIRST_POST:
        m_pcSettings->setValue(SETTING_NAMES[nameIndex(eSETTING_NAME::SETTING_FIRST_POST)], strValue);
        return;
    }

    qDebug() << QString{__FILE__} + " " + QString{__LINE__} + ": Unknown setting name.";
}

/**
 * @brief Settings::ignoreCategories
 * @return
 */
QStringList Settings::ignoreCategories() {
    static const auto STR_IGNORE_CATEGORIES = "IGNORE_CATEGORIES";
    static const auto STR_IGNORE_CATEGORY_NAME = "NAME";

    QStringList lstIgnoreCategories;
    auto nCount = m_pcSettings->beginReadArray(STR_IGNORE_CATEGORIES);
    for (auto i = 0; i < nCount; ++i) {
        m_pcSettings->setArrayIndex(i);
        auto str = m_pcSettings->value(STR_IGNORE_CATEGORY_NAME).toString();
        str = str.trimmed();
        if (!str.isEmpty())
            lstIgnoreCategories.append(str);
    }
    m_pcSettings->endArray();
    if (lstIgnoreCategories.isEmpty()) {
        lstIgnoreCategories << "運営案内" << "他のサイト" << "特別企画";
        m_pcSettings->beginWriteArray(STR_IGNORE_CATEGORIES);
        for (auto i = 0; i < lstIgnoreCategories.size(); ++i) {
            m_pcSettings->setArrayIndex(i);
            m_pcSettings->setValue(STR_IGNORE_CATEGORY_NAME, lstIgnoreCategories.at(i));
        }
        m_pcSettings->endArray();
    }

    return lstIgnoreCategories;
}

/**
 * @brief Settings::ignoreThreads
 * @return
 */
QStringList Settings::ignoreThreads() {
    static const auto STR_IGNORE_THREADS = "IGNORE_THREADS";
    static const auto STR_IGNORE_THREAD_NAME = "NAME";

    QStringList lstIgnoreThreads;
    auto nCount = m_pcSettings->beginReadArray(STR_IGNORE_THREADS);
    for (auto i = 0; i < nCount; ++i) {
        m_pcSettings->setArrayIndex(i);
        auto str = m_pcSettings->value(STR_IGNORE_THREAD_NAME).toString();
        str = str.trimmed();
        if (!str.isEmpty())
            lstIgnoreThreads.append(str);
    }
    m_pcSettings->endArray();
    if (lstIgnoreThreads.isEmpty()) {
        lstIgnoreThreads << "2NN+" << "5chプロジェクト" << "いろいろランク" << "TOPページ" << "RONIN" << "PINK update";
        m_pcSettings->beginWriteArray(STR_IGNORE_THREADS);
        for (auto i = 0; i < lstIgnoreThreads.size(); ++i) {
            m_pcSettings->setArrayIndex(i);
            m_pcSettings->setValue(STR_IGNORE_THREAD_NAME, lstIgnoreThreads.at(i));
        }
        m_pcSettings->endArray();
    }

    return lstIgnoreThreads;
}

/**
 * @brief Settings::makeDataFilePath
 * @return
 */
QString Settings::makeDataFilePath() {
    QString strBbsmenuFilePath;
    auto clstBbsmenuDataPaths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
    if (clstBbsmenuDataPaths.count() >= 0)
        strBbsmenuFilePath = clstBbsmenuDataPaths[0];
    else
        strBbsmenuFilePath = qApp->applicationDirPath();

    return strBbsmenuFilePath;
}

/**
 * @brief   Settings::nameIndex
 *          enum値のインデックスを返す
 * @param[in]   e   インデックスを求めるenum値
 * @return  算出されたenum値
 */
int Settings::nameIndex(const Settings::eSETTING_NAME e) const {
    switch (e) {
    case eSETTING_NAME::SETTING_DATA_FILE_PATH:
        return 0;
    case eSETTING_NAME::SETTING_BBSMENU_FILE_NAME:
        return 1;
    case eSETTING_NAME::SETTING_BBSMENU_URL:
        return 2;
    case eSETTING_NAME::SETTING_VIEW_FONT_NAME:
        return 3;
    case eSETTING_NAME::SETTING_FIRST_POST:
        return 4;
    default:
        return -1;
    }
}
