/**
 * Copyright (c) 2012 Nokia Corporation.
 * Copyright (c) 2012 Harald Meyer, http://www.meh.at
 */

#include "customcamera.h"

#include <QtGui>
#include <QCameraImageCapture>
#include <QDesktopServices>

#include "videosurface.h"
#include "fipthread.h"


/*!
  Constructor.
*/
CustomCamera::CustomCamera(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_camera(0),
      m_cameraImageCapture(0),
      m_videoSurface(0)
{
    // Important, otherwise the paint method is never called
    setFlag(QGraphicsItem::ItemHasNoContents, false);

    // Initialize background processing thread. The thread is started later when data is available.
    m_fipThread = new FIPThread(this);
    connect(m_fipThread, SIGNAL(newFrameReady()), this, SLOT(processedFrameAvailable()));
    connect(m_fipThread, SIGNAL(fullImageSaved(QString)), this, SLOT(fullImageSaved(QString)));
}


/*!
  Destructor.
*/
CustomCamera::~CustomCamera()
{
    destroyResources();
}


/*!
  Releases and destroyes all camera resources.
*/
void CustomCamera::destroyResources()
{
    if (m_camera) {
        m_camera->stop();
    }

    if (m_cameraImageCapture) {
        delete m_cameraImageCapture;
        m_cameraImageCapture = 0;
    }

    delete m_camera;
    m_camera = 0;
}

/*!
  Triggers redrawing.
*/

void CustomCamera::processedFrameAvailable()
{
    update();
}

/*!
  Paints the camera view finder frame. The frame is pulled from the worker thread.
*/
void CustomCamera::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *option,
                         QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_fipThread)
        return;

    // Get processed image from worker thread and draw it.
    QImage *ptrImage = m_fipThread->getLatestProcessedImage();

    if (ptrImage)
    {
        QPointF upperLeft = boundingRect().center() -
                QPointF(ptrImage->width() / 2,
                        ptrImage->height() / 2);


        // Draw the black borders.
        painter->fillRect(0, 0, upperLeft.x(), boundingRect().height(),
                          Qt::black);
        painter->fillRect(upperLeft.x() + ptrImage->width(), 0,
                          boundingRect().right(), boundingRect().bottom(),
                          Qt::black);

        painter->drawImage(QRect(upperLeft.x(), upperLeft.y(),
                                 ptrImage->width(),
                                 ptrImage->height()), *ptrImage);

        // unlock
        m_fipThread->getLatestProcessedImageReady();
    }
    else {
        painter->fillRect(boundingRect(), Qt::black);
    }
}


/*!
  Copies the frame data to the worker thread.
  Returns false when there is error, otherwise returns true.
*/
bool CustomCamera::updateFrame(const QVideoFrame &frame)
{
    if (!frame.isValid()) {
        qDebug() << "CustomCameras::updateFrame: Invalid frame";
        return false;
    }

    if (!m_fipThread || m_fipThread->isProcessing()) {
        // Discard frame if worker thread is busy.
        return true;
    }

    QVideoFrame f = frame;
    if (f.map(QAbstractVideoBuffer::ReadOnly)) {
        m_fipThread->setNewFrame(&f); // send frame to worker thread
        f.unmap(); // ready for next frame from camera
    }

     return true;
}


/*!
  Starts the camera with the given device. The available devices
  can be queried with the availableDevices method. Starting will release
  and destroy all earlier camera resources before creating new ones.
*/
void CustomCamera::start(const QString &device)
{
    destroyResources();

    m_camera = new QCamera(device.toLatin1(), this);

    // Make sure the camera is in loaded state.
    m_camera->load();

    m_videoSurface = new VideoSurface(this, m_camera);
    m_camera->setViewfinder(m_videoSurface);

    // Set the image capturing objects.
    m_cameraImageCapture = new QCameraImageCapture(m_camera);
    m_cameraImageCapture->setCaptureDestination(
                QCameraImageCapture::CaptureToBuffer);

    // Camera API
    connect(m_camera, SIGNAL(locked()), this, SIGNAL(locked()));
    connect(m_camera, SIGNAL(lockFailed()), this, SIGNAL(lockFailed()));

    connect(m_camera, SIGNAL(stateChanged(QCamera::State)),
            this, SLOT(cameraStateChanged(QCamera::State)));
    connect(m_camera, SIGNAL(stateChanged(QCamera::State)),
            this, SIGNAL(cameraStateChanged()));

    // Image capture API
    connect(m_cameraImageCapture, SIGNAL(imageCaptured(int, const QImage&)),
            this, SIGNAL(imageCaptured(int, const QImage&)));

    connect(m_cameraImageCapture, SIGNAL(imageAvailable(int, const QVideoFrame&)),
            this, SLOT(imageAvailable(int, const QVideoFrame&)));

    // We process the image before sending the imageSaved signal:
    //connect(m_cameraImageCapture, SIGNAL(imageSaved(int, QString)),
    //        this, SIGNAL(imageSaved(int, QString)));

    // Set the initial capture mode to image capturing.
    m_camera->setCaptureMode(QCamera::CaptureStillImage);

    // Begin the receiving of view finder frames.
    m_camera->start();
}


/*!
  Stops the camera and releases all resources.
*/
void CustomCamera::stop()
{
    destroyResources();
}


/*!
  Returns the state camera is currently.
*/
CustomCamera::State CustomCamera::cameraState() const
{
    if (!m_camera) {
        return UnloadedState;
    }

    return State(m_camera->state());
}


/*!
  Returns the list of available devices. Eg. primary camera and front camera.
*/
QStringList CustomCamera::availableDevices() const
{
    QStringList list;
    foreach (const QByteArray array, QCamera::availableDevices()) {
        list << QString(array);
    }

    return list;
}

/*!
  Returns the current effect threshold.
*/
int CustomCamera::effectThreshold() const
{
    return m_fipThread->getEffectThreshold();
}


/*!
  Set the effect threshold.
*/
void CustomCamera::effectThreshold(int thresh)
{
    m_fipThread->setEffectThreshold(thresh);
}


/*!
  Gets info about the state change. When the camera is loaded we must emit
  the camera API signal in order to QML side to know about the changed camera.
*/
void CustomCamera::cameraStateChanged(QCamera::State state)
{
    if (state == QCamera::ActiveState) {
        // emit state changes here
    }
}


/*!
  Capture image with the currently set attributes.
*/
void CustomCamera::captureImage()
{
    if (!m_cameraImageCapture) {
        return;
    }

    m_cameraImageCapture->capture();
}


/*!
  Reads the captured image buffer (usually EXIF Jpeg) and copies it to the worker thread.
*/
void CustomCamera::imageAvailable(int id, const QVideoFrame &f)
{
    Q_UNUSED(id);

    QVideoFrame frame = f;
    if (frame.map(QAbstractVideoBuffer::ReadOnly))
    {
        m_fipThread->setFullResolutionFrame(&frame);
        frame.unmap();

        //m_camera->stop(); //this should be called depending on what the app does with the processed image
    }
}


/*!
  The full-resolution image has been processed and saved by the worker thread.
  Forward this information (signal) to the Qml view.
*/
void CustomCamera::fullImageSaved(QString fn)
{
    emit imageSaved(0, fn);
    m_fipThread->setLiveMode();
    //m_camera->start(); //this should be called depending on what the app does with the processed image
}


/*!
  Focuses on the given coordinate. Currently the manual focus is not supported
  by the backend.
*/
void CustomCamera::focusToCoordinate(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    if (!m_camera) {
        return;
    }

    m_camera->searchAndLock();
}


/*!
  Unlocks the focus.
*/
void CustomCamera::unlock()
{
    if (!m_camera) {
        return;
    }

    m_camera->unlock();
}
