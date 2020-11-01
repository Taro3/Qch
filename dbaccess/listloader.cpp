#include <QFile>

#include "listloader.h"
#include "common.h"
#include "qchexception.h"

namespace NSDatabase {

ListLoader::ListLoader(QObject *parent) : QObject(parent) {
}

QList<QVariantList> ListLoader::load(const QString filename) {
    // 挿入データファイル読み込み
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOGGING(filename + " open failed.");
        QCH_EXCEPTION("QFile::open failed. " + filename);
    }
    QString fileData = file.readAll();
    file.close();

    // QStringListへ変換
    fileData.remove('\r');                          // CRを削除
    QStringList data = fileData.split('\n');        // LFで分割
    for (int i = data.count() - 1; i >= 0; --i)     // 空を削除 念のため
        if (data[i].isEmpty())
            data.removeAt(i);

    QList<QVariantList> result;                      // 各データをQListに追加
    for (auto line : data) {
        QStringList lineData = line.split(", ");
        QVariantList vl;
        for (auto s : lineData) vl.append(s);
        result.append(vl);
    }

    return result;
}

}   // end of namespace
