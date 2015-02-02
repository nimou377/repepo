#ifndef VIEWFINDERWRAPPER_H
#define VIEWFINDERWRAPPER_H

#include <QDeclarativeItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QCamera>
#include <QPixmap>
#include "videosurface.h"
#include "processingthread.h"

class ViewFinderWrapper : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(int cameraStatus READ cameraStatus NOTIFY cameraStatusChanged);
    Q_PROPERTY(int cameraError READ cameraError NOTIFY cameraErrorChanged);
    Q_PROPERTY(long frameCount READ frameCount NOTIFY frameCountChanged);
    Q_PROPERTY(long processedCount READ processedCount NOTIFY processedCountChanged);
    Q_PROPERTY(bool running READ running NOTIFY runningChanged);
public:
    explicit ViewFinderWrapper(QDeclarativeItem *parent = 0);
    virtual ~ViewFinderWrapper();
signals:
    void cameraStatusChanged(int status);
    void cameraErrorChanged(int error);
    void frameCountChanged(long count);
    void processedCountChanged(long count);
    void runningChanged();
public slots:
    int cameraStatus() const;
    int cameraError() const;
    long frameCount() const;
    long processedCount() const;

    bool running() const;
    void startCamera();
    void stopCamera();

private slots:
    // Camera
    void onStateChanged(QCamera::State state);
    void onStatusChanged(QCamera::Status status);
    void onCameraError(QCamera::Error error);

    // Surface
    void frameReady();

    // ProcessingThread
    void onFrameProcessed();
    void onThreadCongested();

private:
    void reset();
    void startViewFinder();
    void stopViewFinder();

    // From QDeclarativeItem
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


private:
    QCamera* m_camera;
    QCamera::Status m_cameraStatus;
    QCamera::Error m_cameraError;
    bool m_cameraActive;

    VideoSurface m_surface;
    bool m_viewFinderActive;

    QPixmap m_currentFrame;
    long m_receivedFrameCounter;
    long m_processedFrameCounter;

    ProcessingThread* m_processor;
};

#endif // VIEWFINDERWRAPPER_H
