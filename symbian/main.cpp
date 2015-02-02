#include <QApplication>
#include <QDeclarativeContext>
#include "qmlapplicationviewer.h"
#include "viewfinderwrapper.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    qmlRegisterType<ViewFinderWrapper>("Codemancers", 1, 0, "ViewFinder");

    QmlApplicationViewer viewer;
    QDeclarativeContext* context = viewer.rootContext();


    //QCoreApplication::setApplicationVersion(VERSION_NUMBER);
    context->setContextProperty("version_number", QCoreApplication::applicationVersion());
    context->setContextProperty("compiled_on", QVariant(__DATE__));

    viewer.setMainQmlFile(QLatin1String("qml/CamTest/main.qml"));
    viewer.showExpanded();

    return app->exec();
}
