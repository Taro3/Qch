/**
 * @file    xmloutput.cpp
 *
 * @brief   XMLファイルへのデータ出力制御クラス実装
 *
 * @author  Taro.Yamada
 *
 * @date    2019/01/30
 */

#include <QXmlStreamWriter>
#include <QStandardItemModel>
#include <QDebug>

#include <QFile>

#include "xmloutput.h"
#include "qchexception.h"

/**
 * @brief       XmlOutput::XmlOutput
 *              コンストラクタ
 * @param[in]   parent
 */
XmlOutput::XmlOutput(QObject *parent) : QObject(parent) {
}

/**
 * @brief       XmlOutput::outputCategoryList
 *              カテゴリリストをXMLとしてファイル出力する
 * @param[in]   pFile   出力するQFileオブジェクトポインタ
 * @param[in]   pModel  出力するカテゴリデータを保持するQStandardItemModelオブジェクトポインタ
 * @return      処理結果
 * @retval      true    正常終了
 * @retval      false   異常終了
 */
void XmlOutput::outputCategoryList(QFile * const pFile, const QStandardItemModel * const pModel) const {
    if (!pFile || !pModel) {
        qDebug("arguments is nullptr.");
        return;
    }

    // XMLオブジェクト生成
    QXmlStreamWriter xml(pFile);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("categories");

    writeCategoryList(xml, pModel);

    xml.writeEndElement();
    xml.writeEndDocument();
}

/**
 * @brief           XmlOutput::writeCategoryList
 *                  カテゴリリストのXML書出し処理
 * @param[in]       xml     XML出力用オブジェクト
 * @param[in]       pModel  出力するカテゴリ情報を保持するQStandardItemModelオブジェクト
 */
void XmlOutput::writeCategoryList(QXmlStreamWriter &xml, const QStandardItemModel * const pModel) const {
    Q_ASSERT(pModel);

    // カテゴリのXMLへの出力
    xml.writeAttribute("count", QString::number(pModel->rowCount()));   // カテゴリ数

    // 各カテゴリデータ出力
    for (auto i = 0; i < pModel->rowCount(); ++i) {
        auto pItem = pModel->item(i);

        xml.writeStartElement("category");          // 1カテゴリ開始

        // カテゴリ名保存
        writeCategoryName(xml, pModel->item(i));

        // スレッド一覧保存
        writeThreadList(xml, pItem);

        xml.writeEndElement();                      // 1カテゴリ終了
    }
}

/**
 * @brief       XmlOutput::writeCategoryName
 *              カテゴリ名を出力する
 * @param[in]   xml     XML出力用オブジェクト
 * @param[in]   pItem   出力する1カテゴリ情報を保持するQStandardItemオブジェクトポインタ
 */
void XmlOutput::writeCategoryName(QXmlStreamWriter &xml, const QStandardItem * const pItem) const {
    Q_ASSERT(pItem);

    // 親アイテムとして保持しているカテゴリ名を出力する
    auto strCategoryName = pItem->text();
    xml.writeAttribute("name", strCategoryName);
    xml.writeAttribute("count", QString::number(pItem->rowCount()));
}

/**
 * @brief       XmlOutput::writeThreadList
 *              カテゴリ内のスレッド一覧出力処理
 * @param[in]   xml     XML出力用オブジェクト
 * @param[in]   pItem   出力する1カテゴリ情報を保持するQStandardItemオブジェクトポインタ
 */
void XmlOutput::writeThreadList(QXmlStreamWriter &xml, const QStandardItem * const pItem) const {
    Q_ASSERT(pItem);

    // pItemの子として保持しているスレッドタイトルとスレッドURLを出力する
    for (auto i = 0; i < pItem->rowCount(); ++i) {
        auto pThreadItem = pItem->child(i);
        auto strTitle     = pThreadItem->text();
        auto strUrl       = pThreadItem->data().toString();
        xml.writeTextElement("title", strTitle);
        xml.writeTextElement("url", strUrl);
    }
}
