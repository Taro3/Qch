#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

/**
 * @brief The Settings class
 */
class Settings : public QObject {
    Q_OBJECT
public:
    /**
     * @brief The eSETTING_NAME enum
     */
    enum class eSETTING_NAME {
        SETTING_DATA_FILE_PATH,     //!< データファイルのパス
        SETTING_BBSMENU_FILE_NAME,  //!< bbsmenuのファイル名
        SETTING_BBSMENU_URL,        //!< bbsmenuのURL
        SETTING_VIEW_FONT_NAME,     //!< スレッドビューのフォント名
        SETTING_FIRST_POST,         //!< 初書込状態
    };

    static const QString SETTING_NAMES[];

    explicit    Settings(QObject *parent = nullptr);
    virtual     ~Settings();
    QString     value(const eSETTING_NAME eSettingId);
    void        setValue(const eSETTING_NAME eSettingId, const QString strValue);
    QStringList ignoreCategories();
    QStringList ignoreThreads();

signals:

public slots:

private:
    QSettings   *m_pcSettings;              //!< 設定データ
    static QString  m_strConfigFilePathName;

    QString makeDataFilePath();
    int nameIndex(const eSETTING_NAME e) const;
};

#endif // SETTINGS_H
