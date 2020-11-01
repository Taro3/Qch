/**
 *
 * @file    dbaccess.cpp
 *
 * @brief   SQLite処理クラス実装
 *
 * @author  Taro.Yamada
 *
 * @date    2020/08/07
 *
 */

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>
#include <QDebug>

#include "common.h"
#include "dbaccess.h"
#include "qchexception.h"
#include "listloader.h"
#include "dbutil.h"

namespace NSDatabase {

DBAccess::DBAccess(QObject *parent) : QObject(parent), _database{ QSqlDatabase::addDatabase("QSQLITE") } {
}

DBAccess::~DBAccess() {
    _database.close();
}

void DBAccess::InitDatabase() {
    static const QString DB_NAME = "Qch.db";

    if (!_database.isValid()) {
        LOGGING("addDatabase failed.");
        QCH_EXCEPTION("QSqlDatabase::addDatabase failed");
    }
    _database.setDatabaseName(DB_NAME);
    if (!_database.open()) {
        LOGGING("Database open failed.");
        QCH_EXCEPTION("QSqlDatabase::open failed. " + _database.lastError().text());
    }

    CreateAllTables();  // テーブルの生成
    InsertAllData();    // 初期データ挿入
    /// @todo 実装追加
}

const QList<TABLE_INFO> DBAccess::TABLES = {
    { "ignore_categories_info_table", { "id INTEGER PRIMARY KEY AUTOINCREMENT", "name TEXT" } },    // 無視カテゴリ名
    /// @todo テーブル一覧データ追加
};

const QList<QStringList> DBAccess::INITIAL_DATA = {
    { "ignore_category.csv", TABLES[0].tableName }, // 無視カテゴリリスト
    /// @todo 初期テーブルデータ定義追加
};

void DBAccess::CreateAllTables() {
    // テーブル一覧定義に沿ってテーブルを生成する
    for (auto info : TABLES)
        DbUtil::CreateTable(_database, info.tableName, info.fieldStrs);
}

void DBAccess::InsertAllData() {
    // 全初期データをテーブルに設定する
    for (auto item : INITIAL_DATA)
        InsertInitialTableData(item[1], item[0]);   // テーブル名とデータファイル名を渡す
}

void DBAccess::InsertInitialTableData(const QString tableName, const QString dataFilename)
{
    static const QString QUERY = "INSERT INTO %1 (%2) VALUES('%3')";

    if (DbUtil::RecordCount(_database, tableName) != 0) {               // すでにデータがあるので初期値は処理しない
        LOGGING(tableName + " is already data");
        return;
    }

    QStringList fields = DbUtil::TableFieldNames(_database, tableName); // 指定のテーブルのフィールド名一覧を取得する
    QList<QVariantList> data = ListLoader::load(dataFilename);          // 挿入データファイル読み込みQStringListへ変換
    DbUtil::InsertData(_database, tableName, fields, data);             // データベースへ挿入
}

}   // end of namespace
