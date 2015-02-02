/**
 * Copyright (c) 2012 Harald Meyer, http://www.meh.at
 */

#include "fipmain.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Start the Qml view
    FIPMain main;
    main.show();

    return app.exec();
}
