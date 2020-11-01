/**
 * @file    xmloutput.h
 *
 * @brief   XMLファイルへのデータ出力制御クラスヘッダ
 *
 * @author  Taro.Yamada
 *
 * @date    2019/01/30
 */

#ifndef XMLOUTPUT_H
#define XMLOUTPUT_H

#include <QObject>
#include <QXmlStreamWriter>

class QFile;
class QStandardItemModel;
class QStandardItem;

/**
 * @brief   The XmlOutput class
 */
class XmlOutput : public QObject {
    Q_OBJECT

public:
    explicit XmlOutput(QObject *parent = nullptr);
    void outputCategoryList(QFile * const pFile, const QStandardItemModel * const pModel) const;

signals:

public slots:

private:
    void writeCategoryList(QXmlStreamWriter &xml, const QStandardItemModel * const pModel) const;
    void writeCategoryName(QXmlStreamWriter &xml, const QStandardItem * const pItem) const;
    void writeThreadList(QXmlStreamWriter &xml, const QStandardItem * const pItem) const;
};

#endif // XMLOUTPUT_H
