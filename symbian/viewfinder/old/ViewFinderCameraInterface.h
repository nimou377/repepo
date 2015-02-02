#ifndef VIEWFINDERCAMERAINTERFACE_H
#define VIEWFINDERCAMERAINTERFACE_H

class QPixmap;
class QSize;

// Have to do this the old-fashioned way thru observer instead of signals/slots
// because Symbian-camera, multiple inheritance and moc don't play nice together
class ViewFinderCameraObserver {
public:
    virtual void onCameraError(int error) = 0;
    virtual void onCameraReservationSucceed() = 0;
    virtual void onCameraReservationFailed(int error) = 0;
    virtual void onViewFinderStartFailed(int error) = 0;
    virtual void onViewFinderAlreadyRunning() = 0;
    virtual void onViewFinderStarted() = 0;
    virtual void onViewFinderStopped() = 0;
    virtual void onViewFinderFrameReady() = 0;
};

// base class for different camera implementations
class ViewFinderCamera
{
public:
    virtual ~ViewFinderCamera() {}
    // Mandatory
    virtual void StartViewFinder(QSize size) = 0;
    virtual void StopViewFinder() = 0;
    // Camera own the pixmap, user must take copy
    virtual QPixmap* Frame() = 0;

    // Cleanup method, optional, needed for Symbian
    virtual void Cleanup() {}

    virtual bool hasAutoFocus() { return true; }
};

#endif // VIEWFINDERCAMERAINTERFACE_H
