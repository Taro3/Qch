#ifndef DBUTIL_H
#define DBUTIL_H

#include <QObject>
#include <QSqlDatabase>

namespace NSDatabase {

/**
 * @brief   The DbUtil class
 *          データベースアクセスユーティリティークラス
 */
class DbUtil : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief       DbUtil::DbUtil
     *              コンストラクタ
     * @param[in]   parent  親オブジェクトポインタ
     */
    explicit DbUtil(QObject *parent = nullptr);
    /**
     * @brief       DbUtil::OpenDatabase
     *              データベース作成処理
     * @param[in]   dbName  データベース名
     * @return      作成されたQSqlDatabaseオブジェクト
     */
    static QSqlDatabase OpenDatabase(const QString dbName);
    /**
     * @brief       DbUtil::IsTableExists
     *              テーブルの存在確認
     * @param[in]   db          データベースオブジェクト
     * @param[in]   tableName   確認するテーブル名
     * @return      確認結果
     * @retval      true    存在する
     * @retval      false   存在しない
     */
    static bool IsTableExists(QSqlDatabase& db, const QString tableName);
    /**
     * @brief       DbUtil::RecordCount
     *              テーブルのレコード数取得
     * @param[in]   db          データベースオブジェクト
     * @param[in]   tableName   テーブル名
     * @return      テーブル内のレコード数
     * @note        失敗した場合は例外を送出する
     */
    static int RecordCount(QSqlDatabase& db, const QString tableName);
    /**
     * @brief       DbUtil::CreateTable
     *              テーブルの作成
     * @param[in]   db          データベースオブジェクト
     * @param[in]   tableName   作成するテーブル名
     * @param[in]   fields      作成するフィールドの定義文(SQL形式)
     */
    static void CreateTable(QSqlDatabase& db, const QString tableName, const QStringList fields);
    /**
     * @brief       DbUtil::InsertData
     *              テーブルへのデータ挿入処理
     * @param[in]   db          データベースオブジェクト
     * @param[in]   tableName   テーブル名
     * @param[in]   fields      挿入するフィールド名リスト
     * @param[in]   data        挿入データリスト
     */
    static void InsertData(QSqlDatabase& db, const QString tableName, const QStringList fields, const QList<QList<QVariant> > data);
    /**
     * @brief       DbUtil::TableFieldNames
     *              指定のテーブルのフィールド名一覧を取得する
     * @param[in]   db          データベースオブジェクト
     * @param[in]   tableName   テーブル名
     * @return      フィールド名一覧
     */
    static QStringList TableFieldNames(QSqlDatabase& db, const QString tableName);
    /// @todo レコード取得(SELECT)関数作成

signals:

private:
    static const QString DB_CONNECTION_NAME;    //!< データベース接続名

    /**
     * @brief       openDb
     *              データベースオープン処理
     * @param[in]   db  データベースオオブジェクト
     */
    static void openDb(QSqlDatabase& db);
};

}   // end of namespace

#endif // DBUTIL_H
