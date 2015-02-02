#include "ViewFinderCameraHarmattan.h"

ViewFinderCameraHarmattan::ViewFinderCameraHarmattan(ViewFinderCameraObserver& observer, QObject *parent) :
    QObject(parent), m_observer(observer), m_cameraActive(false), m_viewFinderActive(false)
{
    qDebug() << "[CAMERA] ViewFinderCameraHarmattan::ViewFinderCameraHarmattan";
    m_camera = new QCamera();
    m_focus = m_camera->focus();

    if (m_camera) {
        m_camera->start();
        m_cameraActive = true;
    }
}

ViewFinderCameraHarmattan::~ViewFinderCameraHarmattan()
{
    qDebug() << "[CAMERA] ViewFinderCameraHarmattan::~ViewFinderCameraHarmattan";

    if (m_camera) {
        m_focus = 0;
        m_cameraActive = false;
        m_camera->stop();
        delete m_camera;
        m_camera = 0;
    }
}

void ViewFinderCameraHarmattan::StartViewFinder(QSize size)
{
    qDebug() << "[CAMERA] ViewFinderCameraHarmattan::StartViewFinder";
    // Harmattan viewfinder is fixed to 640x480 by the platform
    if( !m_viewFinderActive ) {
        m_camera->setViewfinder( static_cast<QAbstractVideoSurface*>( &m_surface ) );
        m_viewFinderActive = true;
    }
}

void ViewFinderCameraHarmattan::StopViewFinder()
{
    qDebug() << "[CAMERA] ViewFinderCameraHarmattan::StopViewFinder";
    if( m_viewFinderActive )
    {
        qDebug() << "Stopping viewfinder";
        m_camera->setViewfinder( static_cast<QAbstractVideoSurface*>( 0 ) );
        m_viewFinderActive = false;
    }
}

QPixmap* ViewFinderCameraHarmattan::Frame()
{
    return 0;
}

void ViewFinderCameraHarmattan::error( QCamera::Error value )
{
    qDebug() << "[CAMERA] ViewFinderCameraHarmattan::error:" << value;

    if( m_cameraActive && value != QCamera::NoError )
    {
        StopViewFinder();
        m_observer.onCameraError(value);
    }
}

void ViewFinderCameraHarmattan::stateChanged( QCamera::State state )
{
    qDebug() << "[CAMERA] ViewFinderCameraHarmattan::stateChanged:" << state;
    switch (state) {
    default:
    case QCamera::UnloadedState:
        qDebug() << "[CAMERA] Unloaded state";
        break;
    case QCamera::LoadedState:
        qDebug() << "[CAMERA] Loaded state";
        break;
    case QCamera::ActiveState:
        qDebug() << "[CAMERA] Active state";
        break;
    }
}
