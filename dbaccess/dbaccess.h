/**
 *
 * @file    dbaccess.h
 *
 * @brief   SQLite処理クラスヘッダ
 *
 * @author  Taro.Yamada
 *
 * @date    2020/08/07
 *
 */

#ifndef DBACCESS_H
#define DBACCESS_H

#include <QObject>
#include <QSqlDatabase>

namespace NSDatabase {

/**
 * デーブル定義
 */
typedef struct {
    QString tableName;
    QStringList fieldStrs;
} TABLE_INFO;

/**
 * @brief   The DBAccess class
 *          データベース制御クラス
 */
class DBAccess : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief       DBAccess::DBAccess
     *              コンストラクタ
     * @param[in]   parent  親オブジェクトポインタ
     */
    explicit DBAccess(QObject *parent = nullptr);
    /**
     * @brief   DBAccess::~DBAccess
     *          デストラクタ
     */
    ~DBAccess();
    /**
     * @brief DBAccess::InitDatabase
     *          DBのオープンと初期化を行う
     */
    void InitDatabase();

signals:

private:
    QSqlDatabase _database; //!< データベースオブジェクト
    /**
     * @brief       デーブル一覧定義
     * @note        データ定義は
     *              {テーブル名, {フィールド定義文字列, ...}, ...
     *              のような形
     */
    static const QList<TABLE_INFO> TABLES;
    /**
     * @brief       初期データリスト
     * @note        データ定義は
     *              {データファイル名, テーブル名}, ...
     *              のような形
     */
    static const QList<QStringList> INITIAL_DATA;

    /**
     * @brief   DBAccess::CreateAllTables
     *          全テーブル生成処理
     */
    void CreateAllTables();
    /**
     * @brief       DBAccess::InsertAllData
     *              初期の全テーブルデータを挿入する
     */
    void InsertAllData();
    /**
     * @brief       DBAccess::InsertInitialTableData
     *              各種テーブルに初期値を設定する
     * @param[in]   tableName       データを挿入するテーブル名
     * @param[in]   dataFilename    データリストファイルパス
     * @note        失敗した場合は例外を送出する
     */
    void InsertInitialTableData(const QString tableName, const QString dataFilename);
};

}   // end of namespace

#endif // DBACCESS_H
