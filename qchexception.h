#ifndef QCHEXCEPTION_H
#define QCHEXCEPTION_H

#include <QObject>
#include <QException>

/**
 * @brief   The QchException class
 *          独自例外クラス
 */
class QchException : public QException {
public:
    QchException();
    explicit QchException(const QString strMsg);
    virtual ~QchException();
    virtual QchException *clone() const override;
    virtual void raise() const override;
    void setMessage(const QString message);
    QString message() const;

private:
    QString _message;
};

/**
 * @brief       QCH_EXCEPTION
 *              例外スローマクロ
 * @param[in]   m   例外メッセージ
 */
inline void QCH_EXCEPTION(const QString m) {
    QchException e(QString(__FILE__) + __LINE__ + __FUNCTION__ + m);
    e.raise();
}
/*
#define QCH_EXCEPTION(m) { \
    QchException e(QString(__FILE__) + __LINE__ + __FUNCTION__ + (m)); \
    e.raise(); \
    }
*/
#endif // QCHEXCEPTION_H
