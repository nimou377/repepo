/**
 * Copyright (c) 2012 Harald Meyer, http://www.meh.at
 */

#include "fipmain.h"

#include <QGLWidget>
#include <QDesktopWidget>
#include <QApplication>
#include <QDeclarativeEngine>

#include "customcamera.h"

FIPMain::FIPMain(QObject *parent) :
    QObject(parent)
{
}

/*!
  Show Qml view.
*/
void FIPMain::show()
{
    // Register custom Qml types
    qmlRegisterType<CustomCamera>("CustomElements", 1, 0, "CustomCamera");

    // Init Qml view + performance optimization
    m_qmlView.setSource(QUrl("qrc:/qml/MainView.qml"));

    m_qmlView.setResizeMode(QDeclarativeView::SizeRootObjectToView);


    m_qmlView.setViewport(new QGLWidget());

    m_qmlView.setGeometry(QApplication::desktop()->screenGeometry());

    // Connect signals
    QObject *object = (QObject*)m_qmlView.rootObject();
    QObject::connect(object, SIGNAL(exitApp()), this, SLOT(exitApp()));

    // Show
    m_qmlView.showFullScreen();
}

/*!
  Exit app.
*/
void FIPMain::exitApp()
{
    QApplication::quit();
}
