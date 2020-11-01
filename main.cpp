#include <QApplication>
#include <QMessageBox>

#include "mainwindow.h"
#include "qchexception.h"

/**
 * @brief       main    プログラムメイン関数
 * @param[in]   argc    引数数
 * @param[in]   argv    引数文字列配列
 * @return      プログラム終了コード
 */
int main(int argc, char *argv[])
{
    /// @todo qInstallMessageHandlerを実行してログ出力用ハンドラを登録すること

    try {
        QApplication a(argc, argv);
        MainWindow w;
        w.initSpliiterSize();
        w.show();

        return a.exec();
    } catch(QchException &e) {
        QMessageBox::critical(nullptr, qApp->tr("Exception occured"), e.message());
        return 1;
    }
}
