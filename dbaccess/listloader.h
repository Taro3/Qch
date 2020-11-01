#ifndef LISTLOADER_H
#define LISTLOADER_H

#include <QObject>

namespace NSDatabase {

/**
 * @brief   The ListLoader class
 *          テキストリスト読み込みクラス
 */
class ListLoader : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief       ListLoader::ListLoader
     * @param[in]   parent  親オブジェクトポインタ
     */
    explicit ListLoader(QObject *parent = nullptr);
    /**
     * @brief       ListLoader::load
     *              テキストリストを読み込み、QList<QStringList>に変換する
     *              1行のデータは「,」区切りとする
     * @param[in]   filename    テキストリストファイルパス
     * @return      生成したQStringList
     * @note        読み込み失敗時は例外を送出する
     */
    static QList<QVariantList> load(const QString filename);

signals:

};

}   // end of namespace

#endif // LISTLOADER_H
