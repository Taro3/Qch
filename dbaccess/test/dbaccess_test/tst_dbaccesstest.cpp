#include <QtTest>

// add necessary includes here

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

#include "dbutil.h"
#include "dbaccess.h"
#include "qchexception.h"

using namespace NSDatabase;

/**
 * @brief   The DBAccessTest class
 *          DbUtilユニットテストクラス
 */
class DBAccessTest : public QObject
{
    Q_OBJECT

public:
    DBAccessTest();
    ~DBAccessTest();

private slots:
    // DbUtil class test
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void testOpenDatabaseFail1();
    void testOpenDatabaseFail2();
    void testOpenDatabaseSuccess();
    void testIsTableExsistsFail1();
    void testIsTableExsistsSuccess1();
    void testIsTableExsistsSuccess2();
    void testRecordCountFail1();
    void testRecordCountFail2();
    void testRecordCountFail3();
    void testRecordCountSuccess1();
    void testRecordCountSuccess2();
    void testCreateTableFail1();
    void testCreateTableFail2();
    void testCreateTableFail3();
    void testCreateTableSuccess1();
    void testCreateTableSuccess2();
    void testInsertDataFail1();
    void testInsertDataFail2();
    void testInsertDataFail3();
    void testInsertDataSuccess1();
    void testInsertDataSuccess2();
    void testTableFieldNamesFail1();
    void testTableFieldNamesFail2();
    void testTableFieldNamesSuccess1();

    // DBAccess class test
    void testInitDatabaseSuccess1();

private:
    const QString DB_NAME = "Qch.db";
    const QString DB_CONNECTION_NAME = "QchConnection";

    QSqlDatabase initDb();
};

DBAccessTest::DBAccessTest()
{

}

DBAccessTest::~DBAccessTest()
{

}

void DBAccessTest::initTestCase()
{

}

void DBAccessTest::cleanupTestCase() {
}

void DBAccessTest::init() {
}

void DBAccessTest::cleanup() {
    QSqlDatabase::removeDatabase(DB_CONNECTION_NAME);
    QFile::remove(DB_NAME); // 一旦データベースファイルを削除する
}

void DBAccessTest::testOpenDatabaseFail1() {
    try {
        QSqlDatabase db =  DbUtil::OpenDatabase("");    // データベース名が空
    } catch(QchException& e) {
        QVERIFY(true);  // QchException例外が発生していることを確認
        return;
    }
    QVERIFY(false);
}

void DBAccessTest::testOpenDatabaseFail2() {
    try {
        QSqlDatabase db =  DbUtil::OpenDatabase("*");   // データベース名が不正
    } catch(QchException& e) {
        QVERIFY(true);  // QchException例外が発生していることを確認
        return;
    }
    QVERIFY(false);
}

void DBAccessTest::testOpenDatabaseSuccess() {
    try {
        QSqlDatabase db =  DbUtil::OpenDatabase(DB_NAME);
    }  catch (...) {
        QVERIFY(false); // 例外発生した
        return;
    }

    QVERIFY(QFileInfo::exists(DB_NAME));    // DBファイルの作成確認
}

void DBAccessTest::testIsTableExsistsFail1() {
    try {
        QSqlDatabase db;
        DbUtil::IsTableExists(db, "testtable"); // 設定していないDBを指定
    }  catch (const QchException& e) {
        QVERIFY(true);
        return;
    };
    QVERIFY(false);
}

void DBAccessTest::testIsTableExsistsSuccess1() {
    bool result;
    try {
        QSqlDatabase db = initDb();
        result = DbUtil::IsTableExists(db, "testtable"); // 存在しないテーブルを指定
    } catch (const QchException& e) {
        QVERIFY(false);
        return;
    };

    QVERIFY(result == false);
}

void DBAccessTest::testIsTableExsistsSuccess2() {
    bool result;
    try {
        QSqlDatabase db = initDb();
        db.exec("CREATE TABLE testtable(id INTEGER)");
        result = DbUtil::IsTableExists(db, "testtable"); // 存在するテーブルを指定
    } catch (const QchException& e) {
        QVERIFY(false);
        return;
    };

    QVERIFY(result == true);
}

void DBAccessTest::testRecordCountFail1() {
    try {
        QSqlDatabase db;
        DbUtil::RecordCount(db, "testtable");   // 無効なデータベース指定
    }  catch (const QchException& e) {
        QVERIFY(true);
        return;
    };
    QVERIFY(false);
}

void DBAccessTest::testRecordCountFail2() {
    try {
        QSqlDatabase db = initDb();
        DbUtil::RecordCount(db, "");   // テーブル名が空文字列
    }  catch (const QchException& e) {
        QVERIFY(true);
        return;
    };
    QVERIFY(false);
}

void DBAccessTest::testRecordCountFail3() {
    try {
        QSqlDatabase db = initDb();
        DbUtil::RecordCount(db, "notable");   // テーブルが存在しない
    }  catch (const QchException& e) {
        QVERIFY(true);
        return;
    };
    QVERIFY(false);
}

void DBAccessTest::testRecordCountSuccess1() {
    int result = 999;
    try {
        QSqlDatabase db = initDb();
        db.exec("CREATE TABLE testtable (id integer)");
        result = DbUtil::RecordCount(db, "testtable");  // 0レコードテーブル
    }  catch (const QchException& e) {
        QVERIFY(false);
        return;
    };
    QCOMPARE(result, 0);
}

void DBAccessTest::testRecordCountSuccess2() {
    int result = 0;
    try {
        QSqlDatabase db = initDb();
        db.exec("CREATE TABLE testtable (id integer)");
        db.exec("INSERT INTO testtable (id) values(1)");
        db.exec("INSERT INTO testtable (id) values(2)");
        db.exec("INSERT INTO testtable (id) values(3)");
        result = DbUtil::RecordCount(db, "testtable");   // 3レコードテーブル
    }  catch (const QchException& e) {
        QVERIFY(false);
        return;
    };
    QCOMPARE(result, 3);
}

void DBAccessTest::testCreateTableFail1()
{
    try {
        QSqlDatabase db;
        QStringList fields;
        DbUtil::CreateTable(db, "testtable", fields);   // 無効なDB
    } catch (const QchException&) {
        QVERIFY(true);
        return;
    }
    QVERIFY(false);
}

void DBAccessTest::testCreateTableFail2()
{
    try {
        QSqlDatabase db = initDb();
        QStringList fields = { "id INTEGER", "name TEXT" };
        DbUtil::CreateTable(db, "", fields);   // 無効なテーブル名
    } catch (const QchException&) {
        QVERIFY(true);
        return;
    }
    QVERIFY(false);
}

void DBAccessTest::testCreateTableFail3()
{
    try {
        QSqlDatabase db = initDb();
        QStringList fields;
        DbUtil::CreateTable(db, "testtable", fields);   // 無効なフィールド数
    } catch (const QchException&) {
        QVERIFY(true);
        return;
    }
    QVERIFY(false);
}

void DBAccessTest::testCreateTableSuccess1()
{
    QSqlDatabase db = initDb();
    try {
        QStringList fields = { "id INTEGER", };
        DbUtil::CreateTable(db, "testtable", fields);   // フィールド数1
    } catch (const QchException&) {
        QVERIFY(false);
        return;
    }
    QSqlQuery r = db.exec("PRAGMA table_info('testtable')");
    r.first();
    if (r.record().value("cid").toInt() == 0
        && r.record().value("name").toString() == "id"
        && r.record().value("type").toString() == "INTEGER") {
        QVERIFY(true);
        return;
    }
    QVERIFY(false);
}

void DBAccessTest::testCreateTableSuccess2()
{
    QSqlDatabase db = initDb();
    try {
        QStringList fields = { "id INTEGER", "name TEXT", };
        DbUtil::CreateTable(db, "testtable", fields);   // フィールド数2
    } catch (const QchException&) {
        QVERIFY(false);
        return;
    }
    QSqlQuery r = db.exec("PRAGMA table_info('testtable')");
    r.first();
    if (r.record().value("cid").toInt() == 0
        && r.record().value("name").toString() == "id"
        && r.record().value("type").toString() == "INTEGER") {
        r.next();
        if (r.record().value("cid").toInt() == 1
        && r.record().value("name").toString() == "name"
        && r.record().value("type").toString() == "TEXT") {
            QVERIFY(true);
            return;
        }
    }
    QVERIFY(false);
}

void DBAccessTest::testInsertDataFail1()
{
    QSqlDatabase db = initDb();
    QStringList fields = { "id INTEGER", "name TEXT" };
    DbUtil::CreateTable(db, "testtable", fields);   // フィールド数1
    QStringList fielddata = { "id", "name" };
    QList<QVariantList> data = { { 0, "MyName" }, };
    try {
        QSqlDatabase db2;
        DbUtil::InsertData(db2, "testtable", fielddata, data);  // 不正なDBオブジェクト
    } catch (const QchException&) {
        QVERIFY(true);
        return;
    }
    QVERIFY(true);
}

void DBAccessTest::testInsertDataFail2()
{
    QSqlDatabase db = initDb();
    QStringList fields = { "id INTEGER", "name TEXT" };
    DbUtil::CreateTable(db, "testtable", fields);   // フィールド数1
    QStringList fielddata = { "id", "name" };
    QList<QVariantList> data = { { 0, "MyName" }, };
    try {
        DbUtil::InsertData(db, "", fielddata, data);    // 不正なテーブル名
    } catch (const QchException&) {
        QVERIFY(true);
        return;
    }
    QVERIFY(true);
}

void DBAccessTest::testInsertDataFail3()
{
    QSqlDatabase db = initDb();
    QStringList fields = { "id INTEGER", "name TEXT" };
    DbUtil::CreateTable(db, "testtable", fields);   // フィールド数1
    QStringList fielddata = { "id", "xxxxx" };
    QList<QVariantList> data = { { 0, "MyName" }, };
    try {
        DbUtil::InsertData(db, "testtable", fielddata, data);    // 不正なフィールド名
    } catch (const QchException&) {
        QVERIFY(true);
        return;
    }
    QVERIFY(true);
}

void DBAccessTest::testInsertDataSuccess1()
{
    QSqlDatabase db = initDb();
    QStringList fields = { "id INTEGER", "name TEXT" };
    DbUtil::CreateTable(db, "testtable", fields);   // フィールド数1
    QStringList fielddata = { "id", "name" };
    QList<QVariantList> data = { { 0, "MyName" }, {1, "YourName" }, };
    int beforeCount = DbUtil::RecordCount(db, "testtable");
    try {
        DbUtil::InsertData(db, "testtable", fielddata, data);    // 不正なフィールド値
    } catch (const QchException&) {
        QVERIFY(false);
        return;
    }
    int afterCount = DbUtil::RecordCount(db, "testtable");
    QCOMPARE(afterCount - beforeCount, 2);
}

void DBAccessTest::testInsertDataSuccess2()
{
    QSqlDatabase db = initDb();
    QStringList fields = { "id INTEGER", "name TEXT" };
    DbUtil::CreateTable(db, "testtable", fields);   // フィールド数2
    QStringList fielddata = { "id", "name" };
    QList<QVariantList> data = { { 0, "MyName" }, {1, "YourName" }, };
    try {
        DbUtil::InsertData(db, "testtable", fielddata, data);
    } catch (const QchException&) {
        QVERIFY(false);
        return;
    }
    // 2レコード目のデータを検証する
    QSqlQuery r = db.exec("SELECT * FROM testtable");
    qDebug() << r.isValid() << r.record().count() << r.record().value(0) << r.record().value(1);
    QCOMPARE(r.record().count(), 2);
    r.seek(1);
    QCOMPARE(r.record().value("id"), 1);
    QCOMPARE(r.record().value("name"), "YourName");
}

void DBAccessTest::testTableFieldNamesFail1()
{
    QSqlDatabase db;
    QStringList result;
    try {
        result = DbUtil::TableFieldNames(db, "testtable");   // 無効なDB
    }  catch (const QchException&) {
        QVERIFY(true);
        return;
    }
    QVERIFY(false);
}

void DBAccessTest::testTableFieldNamesFail2()
{
    QSqlDatabase db = initDb();
    QStringList fields = { "id INTEGER", "name TEXT" };
    DbUtil::CreateTable(db, "testtable", fields);
    QStringList result;
    try {
        result = DbUtil::TableFieldNames(db, "xxxxx");   // 無効なテーブル名
    }  catch (const QchException&) {
        QVERIFY(true);
        return;
    }
    QVERIFY(false);
}

void DBAccessTest::testTableFieldNamesSuccess1()
{
    QSqlDatabase db = initDb();
    QStringList fields = { "id INTEGER", "name TEXT" };
    DbUtil::CreateTable(db, "testtable", fields);
    QStringList result;
    try {
        result = DbUtil::TableFieldNames(db, "testtable");
    }  catch (const QchException&) {
        QVERIFY(false);
        return;
    }
    QCOMPARE(result[0], "id");      // フィールド名確認
    QCOMPARE(result[1], "name");
}

void DBAccessTest::testInitDatabaseSuccess1()
{
    DBAccess dba;

    try {
        dba.InitDatabase();
    }  catch (...) {
        QVERIFY(false); // 例外発生
        return;
    }
    QVERIFY(true);
}


QSqlDatabase DBAccessTest::initDb() {
    QSqlDatabase db = DbUtil::OpenDatabase(DB_NAME);
    return db;
}

QTEST_APPLESS_MAIN(DBAccessTest)

#include "tst_dbaccesstest.moc"
