#ifndef VIEWFINDERCAMERASYMBIAN_H
#define VIEWFINDERCAMERASYMBIAN_H

#include <ecam.h>
#include <ecamadvsettings.h>

#include <QtGui/QPixmap>

#include "ViewFinderCameraInterface.h"

class CViewFinderCamera : public CBase, public MCameraObserver2, public ViewFinderCamera
{
public: 
    static CViewFinderCamera* NewL(ViewFinderCameraObserver& aObserver);
    virtual ~CViewFinderCamera();
    void Cleanup();
public: // From ViewFinderCamera
    virtual void StartViewFinder(QSize size);
    virtual void StopViewFinder();
    virtual QPixmap* Frame();
    virtual bool hasAutoFocus();
private:
    CViewFinderCamera(ViewFinderCameraObserver& aObserver);
    void ConstructL();
public:
    virtual void HandleEvent(const TECAMEvent &aEvent);
    virtual void ViewFinderReady(MCameraBuffer &aCameraBuffer, TInt aError);
    virtual void ImageBufferReady(MCameraBuffer &aCameraBuffer, TInt aError);
    virtual void VideoBufferReady(MCameraBuffer &aCameraBuffer, TInt aError);
private:
    void DoHandleEventL(const TECAMEvent &aEvent);
    void SetAdvancedSettings();
    void SetZoomLevels();
    void SetWhiteBalanceSettings();
    void SetFocusSettings();
    void SetFocusRangeSettings();
    void SetAutoFocusSettings();
    void SetStabilizationSettings();
    void PrepareL();
    CCamera::TFormat ImageFormatMax() const;
    TInt FindLargestImageIndex(CCamera::TFormat aFormat);
    QPixmap getPixmapFromCameraL(MCameraBuffer &aCameraBuffer);
    void printSupportedModes();
private:
    ViewFinderCameraObserver& iObserver;
    CCamera* iCamera;
    CCamera::CCameraAdvancedSettings* iCameraAdvancedSettings;
    TCameraInfo iCameraInfo;
    QPixmap iFrame;
    bool iCameraReserved;
    bool iCameraPowered;
    bool iPrepared;
    bool iCleanupDone;
    bool iHasAutoFocus;
    long iFrameCounter;
};

#endif /*VIEWFINDERCAMERASYMBIAN_H*/
