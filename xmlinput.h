/**
 * @file    xmlinput.h
 *
 * @details XMLファイル入力処理クラスヘッダ
 *
 * @author  Taro.Yamada
 *
 * @date    2019/01/30
 */

#ifndef XMLINPUT_H
#define XMLINPUT_H

#include <QObject>

/**
 * @brief The XmlInput class
 */
class XmlInput : public QObject {
    Q_OBJECT
public:
    explicit XmlInput(QObject *parent = nullptr);

signals:

public slots:
};

#endif // XMLINPUT_H
