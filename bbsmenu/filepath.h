/**
 * @file    filepath.h
 *
 * @brief   板一覧(メニュー一覧)ファイルパス取得クラスヘッダ
 *
 * @author  Taro.Yamada
 *
 * @date    2019/09/17
 */

#ifndef FILEPATH_H
#define FILEPATH_H

#include <QObject>

namespace bbsmenu {

/**
 * @brief The FilePath class
 */
class FilePath : public QObject {
    Q_OBJECT
public:
    explicit FilePath(QObject *parent = nullptr);
    static QString bbsmenuFilePathName();

signals:

public slots:
};

}   // end of namespace

#endif // FILEPATH_H
