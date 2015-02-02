/**
 * Copyright (c) 2012 Nokia Corporation.
 * Copyright (c) 2012 Harald Meyer, http://www.meh.at
 */

#ifndef CUSTOMCAMERA_H
#define CUSTOMCAMERA_H

#include <QCamera>
#include <QDeclarativeItem>
#include <QMediaRecorder>
#include <QVideoFrame>

class QCameraImageCapture;
class VideoSurface;
class FIPThread;


// Observer design pattern to provide the view finder frame for the CustomCamera
// object.
class FrameObserver {
public:
    virtual bool updateFrame(const QVideoFrame &frame) = 0;
};


class CustomCamera : public QDeclarativeItem, public FrameObserver
{
    Q_OBJECT
    Q_ENUMS(State)

    // State properties
    Q_PROPERTY(State cameraState READ cameraState NOTIFY cameraStateChanged)

    // Devices properties
    Q_PROPERTY(QStringList availableDevices READ availableDevices)

    // Effect properties
    Q_PROPERTY(int effectThreshold READ effectThreshold WRITE effectThreshold)

public:

    enum State {
        ActiveState = QCamera::ActiveState,
        LoadedState = QCamera::LoadedState,
        UnloadedState = QCamera::UnloadedState
    };

    explicit CustomCamera(QDeclarativeItem *parent = 0);
    virtual ~CustomCamera();

    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget);

    virtual bool updateFrame(const QVideoFrame &frame);

protected slots:
    void cameraStateChanged(QCamera::State state);

public slots:
    void start(const QString &device);
    void stop();

    void captureImage();

    void focusToCoordinate(int x, int y);
    void unlock();

    void processedFrameAvailable();

    void effectThreshold(int thresh);

    void imageAvailable(int id, const QVideoFrame &image);

    void fullImageSaved(QString fn);

signals:

    void locked();
    void lockFailed();

    void imageCaptured(int id, const QImage &image);
    void imageSaved(int id, const QString &fileName);

    void cameraStateChanged();

protected:
    void destroyResources();

    State cameraState() const;

    QStringList availableDevices() const;

    int effectThreshold() const;

protected:
    QCamera *m_camera; // Owned

    // Background image processing
    FIPThread *m_fipThread; // Owned

    // Still image capturing
    QCameraImageCapture *m_cameraImageCapture; // Owned

    // For showing view finder image
    VideoSurface *m_videoSurface; // Owned
};

QML_DECLARE_TYPE(CustomCamera)

#endif // CUSTOMCAMERA_H
