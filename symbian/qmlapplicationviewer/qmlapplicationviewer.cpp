
#include "qmlapplicationviewer.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QApplication>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>

#include <qplatformdefs.h> // MEEGO_EDITION_HARMATTAN


class QmlApplicationViewerPrivate
{
    QString mainQmlFile;
    friend class QmlApplicationViewer;
    static QString adjustPath(const QString &path);
};

QString QmlApplicationViewerPrivate::adjustPath(const QString &path)
{

    return path;
}

QmlApplicationViewer::QmlApplicationViewer(QWidget *parent)
    : QDeclarativeView(parent)
    , d(new QmlApplicationViewerPrivate())
{
    connect(engine(), SIGNAL(quit()), SLOT(close()));
    setResizeMode(QDeclarativeView::SizeRootObjectToView);
    // Qt versions prior to 4.8.0 don't have QML/JS debugging services built in
}

QmlApplicationViewer::~QmlApplicationViewer()
{
    delete d;
}

QmlApplicationViewer *QmlApplicationViewer::create()
{
    return new QmlApplicationViewer();
}

void QmlApplicationViewer::setMainQmlFile(const QString &file)
{
    d->mainQmlFile = QmlApplicationViewerPrivate::adjustPath(file);
    setSource(QUrl::fromLocalFile(d->mainQmlFile));
}

void QmlApplicationViewer::addImportPath(const QString &path)
{
    engine()->addImportPath(QmlApplicationViewerPrivate::adjustPath(path));
}

void QmlApplicationViewer::setOrientation(ScreenOrientation orientation)
{

    Qt::WidgetAttribute attribute;
    switch (orientation) {
    case ScreenOrientationLockPortrait:
        //attribute = Qt::WA_LockPortraitOrientation;
        break;
    case ScreenOrientationLockLandscape:
      //  attribute = Qt::WA_LockLandscapeOrientation;
        break;
    default:
    case ScreenOrientationAuto:
      //  attribute = Qt::WA_AutoOrientation;
        break;

    };
    setAttribute(attribute, true);
}

void QmlApplicationViewer::showExpanded()
{
    show();

}

QApplication *createApplication(int &argc, char **argv)
{
    return new QApplication(argc, argv);

}
