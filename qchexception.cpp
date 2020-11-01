#include "qchexception.h"

/**
 * @brief   QchException::QchException
 *          コンストラクタ
 */
QchException::QchException() : QException{}, _message{QString()} {
}

/**
 * @brief   QchException::QchException
 *          コンストラクタ
 * @param[in]   strMsg  例外メッセージ
 */
QchException::QchException(const QString strMsg) : QException{}, _message{strMsg} {
}

/**
 * @brief   QchException::~QchException
 *          デストラクタ
 */
QchException::~QchException() {
}

/**
 * @brief   QchException::clone
 *          コピー関数
 * @return  新規に生成した自クラスオブジェクト
 */
QchException *QchException::clone() const {
    return new QchException{*this};
}

/**
 * @brief   QchException::raise
 *          例外創出閭里
 */
void QchException::raise() const {
    throw *this;
}

/**
 * @brief       QchException::setMessage
 *              メッセージの設定
 * @param[in]   message 設定メッセージ文字列
 */
void QchException::setMessage(const QString message) {
    _message = message;
}

/**
 * @brief   QchException::message
 *          メッセージの取得
 * @return  メッセージ文字列
 */
QString QchException::message() const {
    return _message;
}
