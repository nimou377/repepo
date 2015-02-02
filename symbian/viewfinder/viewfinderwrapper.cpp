#include "viewfinderwrapper.h"
#include <QDebug>

ViewFinderWrapper::ViewFinderWrapper(QDeclarativeItem *parent) :
    QDeclarativeItem(parent),
    m_camera(0), m_cameraStatus(QCamera::UnloadedStatus), m_cameraError(QCamera::NoError),
    m_cameraActive(false), m_viewFinderActive(false), m_receivedFrameCounter(0), m_processedFrameCounter(0),
    m_processor(0)
{

    m_currentFrame = QPixmap();

    // Set flags to get our QML-element to draw
    setFlag(QGraphicsItem::ItemHasNoContents, false);

    // Connect surface to our slot
    connect(&m_surface, SIGNAL(frameAvailable()), this, SLOT(frameReady()));
}

ViewFinderWrapper::~ViewFinderWrapper()
{
    m_processor->deleteLater();
    stopViewFinder();
    stopCamera();
}

int ViewFinderWrapper::cameraStatus() const
{
    return m_cameraStatus;
}

int ViewFinderWrapper::cameraError() const
{
    return m_cameraError;
}

long ViewFinderWrapper::frameCount() const
{
    return m_receivedFrameCounter;
}

long ViewFinderWrapper::processedCount() const
{
    return m_processedFrameCounter;
}

bool ViewFinderWrapper::running() const
{
    return m_viewFinderActive;
}

void ViewFinderWrapper::startCamera()
{
    Q_ASSERT(!m_camera);
    Q_ASSERT(!m_processor);

    m_processor = new ProcessingThread(this);
    connect(m_processor, SIGNAL(frameProcessed()), this, SLOT(onFrameProcessed()));
    connect(m_processor, SIGNAL(queueFull()), this, SLOT(onThreadCongested()));

    m_processor->start();

    m_camera = new QCamera(this);
    if (m_camera) {
        connect(m_camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(onStateChanged(QCamera::State)));
        connect(m_camera, SIGNAL(statusChanged(QCamera::Status)), this, SLOT(onStatusChanged(QCamera::Status)));
        connect(m_camera, SIGNAL(error(QCamera::Error)), this, SLOT(onCameraError(QCamera::Error)));
        m_camera->start();
        m_cameraActive = true;
    }

}

void ViewFinderWrapper::stopCamera()
{
    Q_ASSERT(m_camera);

    // Stop processing thread
    m_processor->stop();
    m_processor->deleteLater();
    m_processor = 0;

    // Stop viewfinder
    stopViewFinder();

    // Stop camera
    m_cameraActive = false;
    m_camera->stop();
    delete m_camera;
    m_camera = 0;

    // Reset state
    reset();
}

void ViewFinderWrapper::reset()
{
    m_cameraStatus = QCamera::UnloadedStatus;
    m_cameraError = QCamera::NoError;
    m_currentFrame = QPixmap();
    m_receivedFrameCounter = 0;
    m_processedFrameCounter = 0;
    update();
}

void ViewFinderWrapper::startViewFinder()
{
    if (!m_viewFinderActive)
    {
        m_camera->setViewfinder( static_cast<QAbstractVideoSurface*>( &m_surface ) );
        m_viewFinderActive = true;
        emit runningChanged();
    }
}

void ViewFinderWrapper::stopViewFinder()
{
    if (m_viewFinderActive)
    {
        m_receivedFrameCounter = 0;
        m_viewFinderActive = false;
        m_camera->setViewfinder( static_cast<QAbstractVideoSurface*>( 0 ) );
        emit runningChanged();
    }
}

void ViewFinderWrapper::onStateChanged(QCamera::State state)
{
}

void ViewFinderWrapper::onStatusChanged(QCamera::Status status)
{
    m_cameraStatus = status;
    if (m_cameraStatus == QCamera::ActiveStatus) {
        startViewFinder();
    }
    emit cameraStatusChanged(m_cameraStatus);
}

void ViewFinderWrapper::onCameraError(QCamera::Error error)
{
    m_cameraError = error;
    emit cameraErrorChanged(m_cameraError);
}

void ViewFinderWrapper::frameReady()
{
    m_receivedFrameCounter++;
    emit frameCountChanged(m_receivedFrameCounter);

    // Get the current frame from the video surface
    QImage frame = m_surface.frame();
    // Add received frame to processing thread for processing
    if (m_processor) {
        m_processor->addFrameToProcessingQueue(frame);
    }

    // And take a copy for ourselves for drawing it on the screen
    m_currentFrame = QPixmap::fromImage(frame);

    // Update the UI
    update();
}

void ViewFinderWrapper::onFrameProcessed()
{
    m_processedFrameCounter++;
    emit processedCountChanged(m_processedFrameCounter);
    update();
}

void ViewFinderWrapper::onThreadCongested()
{

}

void ViewFinderWrapper::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    if (painter)
    {
        QRect rect = option->rect;

        if (m_currentFrame.isNull()) {
            painter->fillRect(rect, Qt::lightGray);
        } else {
            painter->drawPixmap(rect, m_currentFrame);
        }
    }
}
