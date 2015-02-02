#ifndef VIEWFINDERCAMERASIMULATOR_H
#define VIEWFINDERCAMERASIMULATOR_H

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QSize>
#include <QtGui/QPixmap>

#include "ViewFinderCameraInterface.h"

static const int SIMULATOR_VIEWFINDER_FPS = 25;

class ViewFinderCameraSimulator : public QObject, public ViewFinderCamera
{
    Q_OBJECT
public:
    explicit ViewFinderCameraSimulator(ViewFinderCameraObserver& observer, QObject *parent = 0);
    virtual ~ViewFinderCameraSimulator();
signals:

public:
    virtual void StartViewFinder(QSize size);
    virtual void StopViewFinder();
    virtual QPixmap* Frame();

private slots:
    void initTimeout();
    void timeout();
private:
    void nextTestImage();
private:
    ViewFinderCameraObserver& m_observer;
    QPixmap* m_frame;
    QTimer m_timer;
    QSize m_size;
    long m_frameCounter;
    int m_testImageIndex;
};

#endif // VIEWFINDERCAMERASIMULATOR_H
