/**
 * @file    bbsmenusaver.h
 *
 * @brief   板一覧(メニュー一覧)キャッシュデータ保存処理ヘッダ
 *
 * @author  Taro.Yamada
 *
 * @date    2019/09/17
 */

#ifndef BBSMENUSAVER_H
#define BBSMENUSAVER_H

#include <QObject>

class QStandardItemModel;

namespace bbsmenu {

/**
 * @brief The BbsMenuSaver class
 */
class BbsMenuSaver : public QObject {
    Q_OBJECT
public:
    BbsMenuSaver(const QStandardItemModel * const pModel, QObject *parent = nullptr);
    bool save();

signals:

public slots:

private:
    const QStandardItemModel * const _pModel;

    bool makeCategoryFileDir(const QString strFilename) const;
};

}   // end of namespace

#endif // BBSMENUSAVER_H
