#include "mainwindow.h"
#include <QtGlobal>
#include <QApplication>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;

#ifdef Q_OS_ANDROID
    w.showFullScreen();
#else
    // For desktops: place w in the center of the the screen
    QDesktopWidget *dtop = QApplication::desktop();
    w.resize(dtop->width() / 2, dtop->height() / 2);
    w.move( (dtop->width() - w.width()) / 2, (dtop->height() - w.height()) / 2 );
    w.show();
#endif
    
    return app.exec();
}
