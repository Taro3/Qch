#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>

#include "common.h"
#include "qchexception.h"
#include "dbutil.h"

namespace NSDatabase {

const QString DbUtil::DB_CONNECTION_NAME = "QchConnection";

DbUtil::DbUtil(QObject *parent) : QObject(parent) {
}

QSqlDatabase DbUtil::OpenDatabase(const QString dbName) {
    if (dbName.isEmpty()) {
        LOGGING("database name failed. name=" + dbName);
        QCH_EXCEPTION("database name failed. name=" + dbName);
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", DbUtil::DB_CONNECTION_NAME);
    if (!db.isValid()) {
        LOGGING("QSqlDatabase::addDatabase failed.")
        QCH_EXCEPTION("QSqlDatabase::addDatabase failed");
    }
    db.setDatabaseName(dbName);
    if (!db.open()) {
        LOGGING("QSqlDatabase::open failed.");
        QCH_EXCEPTION("QSqlDatabase::open failed. " + db.lastError().text());
    }

    return db;
}

bool DbUtil::IsTableExists(QSqlDatabase &db, const QString tableName) {
    openDb(db);
    if (!db.isValid()) {
        LOGGING("invalid database.");
        QCH_EXCEPTION("invalid database");
    }
    return db.tables().contains(tableName);
}

int DbUtil::RecordCount(QSqlDatabase &db, const QString tableName) {
    static const QString COUNT_QUERY = "SELECT COUNT(*) FROM %1";

    openDb(db);
    QString countQuery = COUNT_QUERY.arg(tableName);
    QSqlQuery countRes = db.exec(countQuery);
    if (countRes.lastError().isValid()) {
        LOGGING("QSqlDatabase::exec failed. " + countRes.lastError().text());
        QCH_EXCEPTION("QSqlDatabase::exec failed.");
    }
    if (!countRes.record().count()) {
        LOGGING("Nothing QSqlDatabase::exec response.");
        QCH_EXCEPTION("Noting QSqlDatabase::exec failed.");
    }

    countRes.first();
    return countRes.record().value(0).toInt();
}

void DbUtil::CreateTable(QSqlDatabase &db, const QString tableName, const QStringList fields) {
    openDb(db);
    if (IsTableExists(db, tableName))
        return;

    QString query = "CREATE TABLE " + tableName + "(" + fields.join(", ") + ")";
    QSqlQuery r = db.exec(query);
    if (r.lastError().isValid()) {
        LOGGING(r.lastError().text());
        QCH_EXCEPTION("QSqlDatabase::exec failed.");
    }
}

void DbUtil::InsertData(QSqlDatabase &db, const QString tableName, const QStringList fields
                        , const QList<QList<QVariant> > data) {
    static const QString QUERY = "INSERT INTO %1 (%2) VALUES(%3)";

    openDb(db);

    QString queryFields = fields.join(", ");
    QStringList queryPlaceholder;
    for (auto s : fields)
        queryPlaceholder.append(":" + s);
    QString querystr = QUERY.arg(tableName).arg(queryFields).arg(queryPlaceholder.join(", "));
    QSqlQuery query(db);
    query.prepare(querystr);
    for (int i = 0; i < data.count(); ++i) {
        const QList<QVariant>& recData = data[i];
        for (int j = 0; j < recData.count(); ++j)   // データをバインド
            query.bindValue(j, recData[j]);
        if (!query.exec()) {    // 1レコードインサート
            LOGGING("QSqlQuery::exec failed. " + query.lastError().text());
            QCH_EXCEPTION("QSqlQuery::exec failed. " + query.lastError().text());
        }
    }
}

QStringList DbUtil::TableFieldNames(QSqlDatabase &db, const QString tableName) {
    static const QString QUERY = "PRAGMA table_info(%1)";

    openDb(db);
    if (!IsTableExists(db, tableName)) {
        LOGGING("table not found");
        QCH_EXCEPTION("table not found");
    }
    QSqlQuery r = db.exec(QUERY.arg(tableName));
    if (r.lastError().isValid()) {
        LOGGING("QSqlDatabase::exec failed " + r.lastError().text());
        QCH_EXCEPTION("QSqlDatabase::exec failed " + r.lastError().text());
    }

    QStringList result;
    while (r.next())
        result.append(r.value("name").toString());
    return result;
}

void DbUtil::openDb(QSqlDatabase &db) {
    if (!db.open()) {
        LOGGING("database open failed.");
        QCH_EXCEPTION("database open failed.");
    }
}

}   // end of namespace
