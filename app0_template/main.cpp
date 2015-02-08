
#include <QApplication>

#include "mainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QGuiApplication::setApplicationDisplayName(MainWindow::tr("main window..."));

    MainWindow mainWindow;

    mainWindow.show();
    return app.exec();
}
