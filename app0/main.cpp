
#include <QApplication>

#include "mainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QGuiApplication::setApplicationDisplayName(MainWindow::tr("main window..."));

    MainWindow mainWindow;
#ifdef Q_OS_WIN
    mainWindow.setFixedSize(800,800);
#endif
#ifdef Q_OS_ANDROID
    mainWindow.setFixedSize(1080,1920-75);
#endif
    mainWindow.show();
    return app.exec();
}
