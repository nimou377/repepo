#ifndef VIEWFINDERCAMERAHARMATTAN_H
#define VIEWFINDERCAMERAHARMATTAN_H

#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QCamera>
#include <QCameraFocusControl>
#include "ViewFinderCameraInterface.h"
#include "harmattanvideosurface.h"

class ViewFinderCameraHarmattan : public QObject, public ViewFinderCamera
{
    Q_OBJECT
public:
    explicit ViewFinderCameraHarmattan(ViewFinderCameraObserver& observer, QObject *parent = 0);
    virtual ~ViewFinderCameraHarmattan();
public:
    // Mandatory
    virtual void StartViewFinder(QSize size);
    virtual void StopViewFinder();
    // Camera own the pixmap, user must take copy
    virtual QPixmap* Frame();
private slots:
    void error( QCamera::Error value );
    void stateChanged( QCamera::State state );
private:
    ViewFinderCameraObserver& m_observer;
    QCamera*                m_camera;
    QCameraFocus*    m_focus;
    HarmattanVideoSurface   m_surface;
    bool m_cameraActive;
    bool m_viewFinderActive;
};

#endif // VIEWFINDERCAMERAHARMATTAN_H
