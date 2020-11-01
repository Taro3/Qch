#ifndef TLDEFINE_H
#define TLDEFINE_H

#include <QObject>

/**
 *  @struct スレッドデータ構造体
 *          タイプIDはグローバルな名前領域に存在する必要があるのでネームスペース外に配置する
 */
typedef struct _THREAD_TITLE_DATA {
    int nThreadNumber;  //!< スレッド番号
    QDateTime cThreadCreateedDateTime;  //!< スレッド生成日時
    QString strThreadUrl;   //!< スレッドURL
    QString strThreadTitle; //!< スレッドタイトル
    int nMessageCount;  //!< レス数
} THREAD_TITLE_DATA;
Q_DECLARE_METATYPE(THREAD_TITLE_DATA)

#endif // TLDEFINE_H
