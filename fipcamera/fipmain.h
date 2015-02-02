/**
 * Copyright (c) 2012 Harald Meyer, http://www.meh.at
 */

#ifndef FIPMAIN_H
#define FIPMAIN_H

#include <QObject>
#include <QDeclarativeView>

/*
 * This class starts the Qml view and handles application exit.
 */

class FIPMain : public QObject
{
    Q_OBJECT
public:
    explicit FIPMain(QObject *parent = 0);
    
Q_SIGNALS:
    
public Q_SLOTS:
    void show();
    void exitApp();

private:
    QDeclarativeView m_qmlView;
    
};

#endif // FIPMAIN_H
