#include "dialog.h"
#include <QApplication>
#include <QSettings>
#include <QPixmap>
#include <QSplashScreen>
#include <QThread>

int main(int argc, char *argv[])
{
    //! 使用 QSettings
    QCoreApplication::setOrganizationName( "YuanLong" );
    QCoreApplication::setApplicationName( "X-FileManager" );

    QApplication a(argc, argv);

    QPixmap pixmap;
    pixmap.load(":/images/SplashScreen.jpg");
    pixmap = pixmap.scaled(QSize(600, 500), Qt::KeepAspectRatioByExpanding);

    QSplashScreen screen(pixmap);
    screen.show();
    screen.setFont(QFont("微软雅黑", 9));
    screen.showMessage("程序启动中...", Qt::AlignHCenter|Qt::AlignBottom, QColor(238,234,157));
    QThread::sleep(2);
    Dialog w;
    w.show();
    screen.finish(&w);

    return a.exec();
}
