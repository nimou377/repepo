#include "ViewFinderCameraSymbian.h"
#include <ecam.h>
#include <ecamadvsettings.h>
#include <ecamadvsettingsuids.hrh>
#include <pathinfo.h>
#include <gdi.h>
#include <s32file.h>
#include <fbs.h>
#include <QtCore/QDebug>



CViewFinderCamera* CViewFinderCamera::NewL(ViewFinderCameraObserver& aObserver)
{
    CViewFinderCamera* self = new (ELeave) CViewFinderCamera(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

CViewFinderCamera::CViewFinderCamera(ViewFinderCameraObserver& aObserver) : iObserver(aObserver),
    iCamera(0), iCameraAdvancedSettings(0),
    iCameraReserved(false), iCameraPowered(false), iPrepared(false), iCleanupDone(false),
    iFrameCounter(0),
    iHasAutoFocus(false)
{
    qDebug() << "[CAMERA] CViewFinderCamera::CViewFinderCamera";
}

CViewFinderCamera::~CViewFinderCamera()
{
    qDebug() << "[CAMERA] CViewFinderCamera::~CViewFinderCamera";

    // Must call Cleanup before deletion
    Q_ASSERT(iCleanupDone);
}

void CViewFinderCamera::Cleanup()
{
    qDebug() << "[CAMERA] Cleanup";

    if (iCameraPowered) {
        qDebug() << "[CAMERA] Camera is powered on, switch it off...";
        iCamera->PowerOff();
    }
    // Release camera from us
    if (iCameraReserved) {
        qDebug() << "[CAMERA] Camera is reserved to us, release it...";
        iCamera->Release();
    }

    delete iCameraAdvancedSettings;
    qDebug() << "[CAMERA] Delete camera...";
    delete iCamera;

    iCleanupDone = true;
}

void CViewFinderCamera::ConstructL()
{
    int cameraCount = CCamera::CamerasAvailable();
    qDebug() << "[CAMERA] Device has "<< cameraCount << "cameras";

    if (cameraCount < 1)
    {
        qDebug() << "[CAMERA][ERROR] No cameras available on device";
        iObserver.onCameraReservationFailed(KErrNotFound);
        return;
    }

    // Create camera and reserve it for us
    qDebug() << "[CAMERA] Create camera...";
    static const int CAMERA_PRIORITY = 50; // -100 to 100
    iCamera = CCamera::New2L(*this, 0, CAMERA_PRIORITY);
    iCameraAdvancedSettings = CCamera::CCameraAdvancedSettings::NewL(*iCamera);
    iCamera->CameraInfo(iCameraInfo);
    qDebug() << "[CAMERA] Reserve camera for us...";
    iCamera->Reserve();
}

void CViewFinderCamera::HandleEvent(const TECAMEvent &aEvent)
{
    TRAPD(err, DoHandleEventL(aEvent););
    if (err != KErrNone) {
        qDebug() << "[CAMERA][ERROR] CViewFinderCamera::HandleEvent err:" << err;
        iObserver.onCameraError(err);
    }
}

void CViewFinderCamera::ViewFinderReady(MCameraBuffer &aCameraBuffer,
                                        TInt aError)
{
    ++iFrameCounter;
    if (aError != KErrNone) {
        qDebug() << "[CAMERA][ERROR] ViewFinderReady error:" << aError;
        iObserver.onCameraError(aError);
        aCameraBuffer.Release();
        return;
    }

    TRAPD(err,
          iFrame = getPixmapFromCameraL(aCameraBuffer);
            );
    if (err != KErrNone) {
        qDebug() << "[CAMERA][ERROR] Error getting frame from camera:" << err;
        iObserver.onCameraError(err);
        aCameraBuffer.Release();
        return;
    }

    iObserver.onViewFinderFrameReady();
    aCameraBuffer.Release();

    static const int FOCUS_EVERY_NTH_FRAME = 45;
    if (iFrameCounter > FOCUS_EVERY_NTH_FRAME) {
        qDebug() << "[CAMERA]" << FOCUS_EVERY_NTH_FRAME << "frames passed, Do focus";
        iFrameCounter = 0;

        // Do focus
        iCameraAdvancedSettings->SetFocusRange(CCamera::CCameraAdvancedSettings::EFocusRangeMacro);
        iCameraAdvancedSettings->SetAutoFocusArea(CCamera::CCameraAdvancedSettings::EAutoFocusTypeSingleArea);
        iCameraAdvancedSettings->SetAutoFocusType(CCamera::CCameraAdvancedSettings::EAutoFocusTypeSingle);
        iCameraAdvancedSettings->SetFocusMode(CCamera::CCameraAdvancedSettings::EFocusModeAuto);
        iCameraAdvancedSettings->SetContinuousAutoFocusTimeout(1);
    }
}

void CViewFinderCamera::ImageBufferReady(MCameraBuffer &aCameraBuffer,
                                         TInt aError)
{
    aCameraBuffer.Release();
}

void CViewFinderCamera::VideoBufferReady(MCameraBuffer &aCameraBuffer,
                                         TInt aError)
{
    TRAPD(err,
          iFrame = getPixmapFromCameraL(aCameraBuffer);
            );
    aCameraBuffer.Release();
}

void CViewFinderCamera::StartViewFinder(QSize size)
{
    qDebug() << "[CAMERA] StartViewFinder: " << size;
    if (!iCameraPowered) {
        qDebug() << "[CAMERA][ERROR] Camera is not powered";
    }
    if (!iPrepared) {
        TRAPD(err, PrepareL());
        if (err != KErrNone) {
            qDebug() << "[CAMERA][ERROR] Error preparing viewfinder:" << err;
            iObserver.onViewFinderStartFailed(err);
            return;
        }
    }
    if (!iCamera->ViewFinderActive()) {
        TSize frameSize;
        frameSize.iWidth = size.width();
        frameSize.iHeight = size.height();
        TRAPD(err, iCamera->StartViewFinderBitmapsL(frameSize));
        if (err != KErrNone) {
            qDebug() << "[CAMERA][ERROR] Could not start viewfinder:" << err;
            iObserver.onViewFinderStartFailed(err);
            return;
        } else {
            qDebug() << "[CAMERA] Start viewfinder in size: " << size.width() << "x" << size.height();
        }
        iObserver.onViewFinderStarted();
    } else {
        iObserver.onViewFinderAlreadyRunning();
    }
}

void CViewFinderCamera::StopViewFinder()
{
    qDebug() << "[CAMERA] StopViewFinder";
    if (iCamera->ViewFinderActive()) {
        iCamera->StopViewFinder();
        iObserver.onViewFinderStopped();
    } else {
        qDebug() << "[CAMERA] Viewfinder was not running";
    }
}

QPixmap* CViewFinderCamera::Frame()
{
    return &iFrame;
}

bool CViewFinderCamera::hasAutoFocus()
{
    TInt supportedFocusTypes = iCameraAdvancedSettings->SupportedAutoFocusTypes();

    return supportedFocusTypes & CCamera::CCameraAdvancedSettings::EFocusModeAuto;
}

void CViewFinderCamera::DoHandleEventL(const TECAMEvent &aEvent)
{
    if (aEvent.iEventType == KUidECamEventReserveComplete)
    {
        if (aEvent.iErrorCode != KErrNone) {
            qDebug() << "[CAMERA][ERROR] Could not reserve camera:" << aEvent.iErrorCode;
            iObserver.onCameraReservationFailed(aEvent.iErrorCode);
            return;
        }
        iCameraReserved = true;
        qDebug() << "[CAMERA] Camera reserved for us";
        qDebug() << "[CAMERA] Power on camera...";
        iCamera->PowerOn();
        return;
    }

    if (aEvent.iEventType == KUidECamEventPowerOnComplete)
    {
        if (aEvent.iErrorCode != KErrNone) {
            qDebug() << "[CAMERA][ERROR] Could not power on camera:" << aEvent.iErrorCode;
            iObserver.onCameraReservationFailed(aEvent.iErrorCode);
            return;
        }
        iCameraPowered = true;

        printSupportedModes();
        SetAdvancedSettings();

        iObserver.onCameraReservationSucceed();
        return;
    }

    if (aEvent.iEventType == KUidECamEventCameraNoLongerReserved)
    {
        qDebug() << "[CAMERA][ERROR] Camera reservation stolen";
        iCameraReserved = false;
        iObserver.onCameraReservationFailed(aEvent.iErrorCode);
    }


    if (aEvent.iEventType == TUid::Uid(KUidECamEventCameraSettingAutoFocusType2UidValue))
    {
        qDebug() << "[CAMERA] KUidECamEventCameraSettingAutoFocusType2UidValue:" << aEvent.iErrorCode;
    }

    if (aEvent.iEventType == TUid::Uid(KUidECamEventCameraSettingFocusRange2UidValue))
    {
        qDebug() << "[CAMERA] KUidECamEventCameraSettingFocusRange2UidValue:" << aEvent.iErrorCode;
    }
}

void CViewFinderCamera::SetAdvancedSettings()
{
    qDebug() << "[CAMERA] SetAdvancedSettings";

    //iCamera->SetBrightnessL(CCamera::EBrightnessAuto); -> -5 cameraerror from these
    //iCamera->SetContrastL(CCamera::EContrastAuto);
    SetZoomLevels();
    SetWhiteBalanceSettings();
    SetStabilizationSettings();
    SetFocusRangeSettings();
    SetFocusSettings();
}

void CViewFinderCamera::SetZoomLevels()
{
    if (iCameraInfo.iMaxZoomFactor > 1.0) {
        int zoomFactor = 2;
        iCamera->SetZoomFactorL(zoomFactor);
        qDebug() << "[CAMERA] Set optical zoom factor to" << zoomFactor;
    } else if (iCameraInfo.iMaxDigitalZoomFactor > 1.0) {
        int zoomFactor = 25;
        iCamera->SetDigitalZoomFactorL(zoomFactor);
        qDebug() << "[CAMERA] Set digital zoom factor to" << zoomFactor;
    }

    qDebug() << "[CAMERA] Current zoom factor:" << iCamera->ZoomFactor();
    qDebug() << "[CAMERA] Current digital zoom factor:" << iCamera->DigitalZoomFactor();
}

void CViewFinderCamera::SetWhiteBalanceSettings()
{
    TInt supportedWBModes = iCameraAdvancedSettings->SupportedWhiteBalanceModes();

    if (supportedWBModes & CCamera::EWBAuto) {
        qDebug() << "[CAMERA][WB] Using CCamera::EWBAuto";
        iCameraAdvancedSettings->SetWhiteBalanceMode(CCamera::EWBAuto);
    } else if (supportedWBModes & CCamera::EWBFluorescent) {
        qDebug() << "[CAMERA][WB] Using CCamera::EWBFluorescent";
        iCameraAdvancedSettings->SetWhiteBalanceMode(CCamera::EWBFluorescent);
    } else if (supportedWBModes & CCamera::EWBTungsten) {
        qDebug() << "[CAMERA][WB] Using CCamera::EWBTungsten";
        iCameraAdvancedSettings->SetWhiteBalanceMode(CCamera::EWBTungsten);
    } else if (supportedWBModes & CCamera::EWBDaylight) {
        qDebug() << "[CAMERA][WB] Using CCamera::EWBDaylight";
        iCameraAdvancedSettings->SetWhiteBalanceMode(CCamera::EWBDaylight);
    } else {
        qDebug() << "[CAMERA][WB] Hmm no mode we want is supported";
    }
}

void CViewFinderCamera::SetFocusSettings()
{
    TInt supportedFocusTypes = iCameraAdvancedSettings->SupportedAutoFocusTypes();

    if (supportedFocusTypes & CCamera::CCameraAdvancedSettings::EFocusModeManual) {
        qDebug() << "[CAMERA][FOCUS] Using CCamera::CCameraAdvancedSettings::EFocusModeManual";
        iCameraAdvancedSettings->SetFocusMode(CCamera::CCameraAdvancedSettings::EFocusModeManual);
    } else if (supportedFocusTypes & CCamera::CCameraAdvancedSettings::EFocusModeAuto) {
        qDebug() << "[CAMERA][FOCUS] Using CCamera::CCameraAdvancedSettings::EFocusModeAuto";
        iCameraAdvancedSettings->SetFocusMode(CCamera::CCameraAdvancedSettings::EFocusModeAuto);
        SetAutoFocusSettings();
    } else if (supportedFocusTypes & CCamera::CCameraAdvancedSettings::EFocusModeFixed) {
        qDebug() << "[CAMERA][FOCUS] Using CCamera::CCameraAdvancedSettings::EFocusModeFixed";
        iCameraAdvancedSettings->SetFocusMode(CCamera::CCameraAdvancedSettings::EFocusModeFixed);
    }

//    SetFocusRangeSettings();
}

void CViewFinderCamera::SetFocusRangeSettings()
{
    TInt supportedFocusRanges = iCameraAdvancedSettings->SupportedFocusRanges();

    //    if (supportedFocusRanges & CCamera::CCameraAdvancedSettings::EFocusRangeSuperMacro) {
    //        qDebug() << "[CAMERA][FOCUSRANGE] Super macro range supported, use it";
    //        iCameraAdvancedSettings->SetFocusRange(CCamera::CCameraAdvancedSettings::EFocusRangeSuperMacro);
    //    } else
    if (supportedFocusRanges & CCamera::CCameraAdvancedSettings::EFocusRangeMacro) {
        qDebug() << "[CAMERA][FOCUSRANGE] Using CCamera::CCameraAdvancedSettings::EFocusRangeMacro";
        iCameraAdvancedSettings->SetFocusRange(CCamera::CCameraAdvancedSettings::EFocusRangeMacro);
    } else if (supportedFocusRanges & CCamera::CCameraAdvancedSettings::EFocusRangeAuto) {
        qDebug() << "[CAMERA][FOCUSRANGE] Using CCamera::CCameraAdvancedSettings::EFocusRangeAuto";
        iCameraAdvancedSettings->SetFocusRange(CCamera::CCameraAdvancedSettings::EFocusRangeAuto);
    } else if (supportedFocusRanges & CCamera::CCameraAdvancedSettings::EFocusRangeNormal) {
        qDebug() << "[CAMERA][FOCUSRANGE] Using CCamera::CCameraAdvancedSettings::EFocusRangeNormal";
        iCameraAdvancedSettings->SetFocusRange(CCamera::CCameraAdvancedSettings::EFocusRangeNormal);
    }
}

void CViewFinderCamera::SetAutoFocusSettings()
{
    TInt supportedAutoFocusTypes = iCameraAdvancedSettings->SupportedAutoFocusTypes();

    static const int AUTOFOCUS_TIMEOUT = 5;

    if (supportedAutoFocusTypes & CCamera::CCameraAdvancedSettings::EAutoFocusTypeContinuous) {
        qDebug() << "[CAMERA][AUTOFOCUS] Using CCamera::CCameraAdvancedSettings::EAutoFocusTypeContinuous";
        iCameraAdvancedSettings->SetAutoFocusType(CCamera::CCameraAdvancedSettings::EAutoFocusTypeContinuous);
        iCameraAdvancedSettings->SetContinuousAutoFocusTimeout(AUTOFOCUS_TIMEOUT);
    } else if (supportedAutoFocusTypes & CCamera::CCameraAdvancedSettings::EAutoFocusTypeSingle) {
        qDebug() << "[CAMERA][AUTOFOCUS] Using CCamera::CCameraAdvancedSettings::EAutoFocusTypeSingle";
        iCameraAdvancedSettings->SetAutoFocusType(CCamera::CCameraAdvancedSettings::EAutoFocusTypeSingle);
    } else  {
        qDebug() << "[CAMERA][AUTOFOCUS] Using CCamera::CCameraAdvancedSettings::EAutoFocusTypeOff";
        iCameraAdvancedSettings->SetAutoFocusType(CCamera::CCameraAdvancedSettings::EAutoFocusTypeOff);
    }

    TInt supportedAutoFocusAreas = iCameraAdvancedSettings->SupportedAutoFocusAreas();

    if (supportedAutoFocusAreas & CCamera::CCameraAdvancedSettings::EAutoFocusTypeSingleArea) {
        qDebug() << "[CAMERA][AUTOFOCUS] Using CCamera::CCameraAdvancedSettings::EAutoFocusTypeSingleArea";
        iCameraAdvancedSettings->SetAutoFocusArea(CCamera::CCameraAdvancedSettings::EAutoFocusTypeSingleArea);
    } else if (supportedAutoFocusAreas & CCamera::CCameraAdvancedSettings::EAutoFocusTypeAuto) {
        qDebug() << "[CAMERA][AUTOFOCUS] Using CCamera::CCameraAdvancedSettings::EAutoFocusTypeAuto";
        iCameraAdvancedSettings->SetAutoFocusArea(CCamera::CCameraAdvancedSettings::EAutoFocusTypeAuto);
    } else if (supportedAutoFocusAreas & CCamera::CCameraAdvancedSettings::EAutoFocusTypeMultiAreaCentered) {
        qDebug() << "[CAMERA][AUTOFOCUS] Using CCamera::CCameraAdvancedSettings::EAutoFocusTypeMultiAreaCentered";
        iCameraAdvancedSettings->SetAutoFocusArea(CCamera::CCameraAdvancedSettings::EAutoFocusTypeMultiAreaCentered);
    }
}


void CViewFinderCamera::SetStabilizationSettings()
{
    TInt supportedStabilModes = iCameraAdvancedSettings->SupportedStabilizationModes();

    if (supportedStabilModes & CCamera::CCameraAdvancedSettings::EStabilizationModeAuto) {
        qDebug() << "[CAMERA][STABIL] Using CCamera::CCameraAdvancedSettings::EStabilizationModeAuto";
        iCameraAdvancedSettings->SetStabilizationMode(CCamera::CCameraAdvancedSettings::EStabilizationModeAuto);
    } else if (supportedStabilModes & CCamera::CCameraAdvancedSettings::EStabilizationModeVertical) {
        qDebug() << "[CAMERA][STABIL] Using CCamera::CCameraAdvancedSettings::EStabilizationModeVertical";
        iCameraAdvancedSettings->SetStabilizationMode(CCamera::CCameraAdvancedSettings::EStabilizationModeVertical);
    } else {
        qDebug() << "[CAMERA][STABIL] Using CCamera::CCameraAdvancedSettings::EStabilizationModeOff";
        iCameraAdvancedSettings->SetStabilizationMode(CCamera::CCameraAdvancedSettings::EStabilizationModeOff);
    }

    TInt supportedStabilEffects = iCameraAdvancedSettings->SupportedStabilizationEffects();
    if (supportedStabilEffects & CCamera::CCameraAdvancedSettings::EStabilizationAuto) {
        qDebug() << "[CAMERA][STABIL] Using CCamera::CCameraAdvancedSettings::EStabilizationAuto";
        iCameraAdvancedSettings->SetStabilizationEffect(CCamera::CCameraAdvancedSettings::EStabilizationAuto);
    } else if (supportedStabilEffects & CCamera::CCameraAdvancedSettings::EStabilizationFine) {
        qDebug() << "[CAMERA][STABIL] Using CCamera::CCameraAdvancedSettings::EStabilizationFine";
        iCameraAdvancedSettings->SetStabilizationEffect(CCamera::CCameraAdvancedSettings::EStabilizationFine);
    } else if (supportedStabilEffects & CCamera::CCameraAdvancedSettings::EStabilizationMedium) {
        qDebug() << "[CAMERA][STABIL] Using CCamera::CCameraAdvancedSettings::EStabilizationMedium";
        iCameraAdvancedSettings->SetStabilizationEffect(CCamera::CCameraAdvancedSettings::EStabilizationMedium);
    } else if (supportedStabilEffects & CCamera::CCameraAdvancedSettings::EStabilizationStrong) {
        qDebug() << "[CAMERA][STABIL] Using CCamera::CCameraAdvancedSettings::EStabilizationStrong";
        iCameraAdvancedSettings->SetStabilizationEffect(CCamera::CCameraAdvancedSettings::EStabilizationStrong);
    }

    TInt supportedStabilComplexities = iCameraAdvancedSettings->SupportedStabilizationComplexityValues();

    if (supportedStabilComplexities & CCamera::CCameraAdvancedSettings::EStabilizationComplexityHigh) {
        qDebug() << "[CAMERA][STABIL] Using CCamera::CCameraAdvancedSettings::EStabilizationComplexityHigh";
        iCameraAdvancedSettings->SetStabilizationComplexity(CCamera::CCameraAdvancedSettings::EStabilizationComplexityHigh);
    } else if (supportedStabilComplexities & CCamera::CCameraAdvancedSettings::EStabilizationComplexityAuto) {
        qDebug() << "[CAMERA][STABIL] Using CCamera::CCameraAdvancedSettings::EStabilizationComplexityAuto";
        iCameraAdvancedSettings->SetStabilizationComplexity(CCamera::CCameraAdvancedSettings::EStabilizationComplexityAuto);
    } else if (supportedStabilComplexities & CCamera::CCameraAdvancedSettings::EStabilizationComplexityMedium) {
        qDebug() << "[CAMERA][STABIL] Using CCamera::CCameraAdvancedSettings::EStabilizationComplexityMedium";
        iCameraAdvancedSettings->SetStabilizationComplexity(CCamera::CCameraAdvancedSettings::EStabilizationComplexityMedium);
    } else if (supportedStabilComplexities & CCamera::CCameraAdvancedSettings::EStabilizationComplexityLow) {
        qDebug() << "[CAMERA][STABIL] Using CCamera::CCameraAdvancedSettings::EStabilizationComplexityLow";
        iCameraAdvancedSettings->SetStabilizationComplexity(CCamera::CCameraAdvancedSettings::EStabilizationComplexityLow);
    }
}

void CViewFinderCamera::PrepareL()
{
    qDebug() << "[CAMERA] PrepareL";
    CCamera::TFormat format = ImageFormatMax();
    TInt idx = FindLargestImageIndex(format);
    iCamera->PrepareImageCaptureL(format, idx);
    iPrepared = true;
}


CCamera::TFormat CViewFinderCamera::ImageFormatMax() const
{
    qDebug() << "[CAMERA] Get image format with most colors";
    if (iCameraInfo.iImageFormatsSupported & CCamera::EFormatFbsBitmapColor16M) {
        qDebug() << "[CAMERA] Image format EFormatFbsBitmapColor16M";
        return CCamera::EFormatFbsBitmapColor16M;
    }
    if (iCameraInfo.iImageFormatsSupported & CCamera::EFormatFbsBitmapColor64K) {
        qDebug() << "[CAMERA] Image format EFormatFbsBitmapColor64K";
        return CCamera::EFormatFbsBitmapColor64K;
    }
    if (iCameraInfo.iImageFormatsSupported & CCamera::EFormatFbsBitmapColor4K) {
        qDebug() << "[CAMERA] Image format EFormatFbsBitmapColor4K";
        return CCamera::EFormatFbsBitmapColor4K;
    }
    qDebug() << "[CAMERA][ERROR] None of our preferred formats are supported, using EFormatMonochrome";
    return CCamera::EFormatMonochrome;
}

TInt CViewFinderCamera::FindLargestImageIndex(CCamera::TFormat aFormat)
{
    qDebug() << "[CAMERA] Find largest supported image size";
    for (TInt i = 0; i < iCameraInfo.iNumImageSizesSupported; i++) {
        TSize size;
        iCamera->EnumerateCaptureSizes(size, i, ImageFormatMax());
        qDebug() << "[CAMERA] i:" << i << ", size:" << size.iWidth << "x" << size.iHeight;
        if (size.iHeight > 0) {
            qDebug() << "[CAMERA] Return" << i;
            return i;
        }
    }
    return 0;
}

QPixmap CViewFinderCamera::getPixmapFromCameraL(MCameraBuffer &aCameraBuffer)
{
    CFbsBitmap& cameraBitmap = aCameraBuffer.BitmapL(0);
    return QPixmap::fromSymbianCFbsBitmap(&cameraBitmap);
}

void CViewFinderCamera::printSupportedModes()
{

#ifdef DEBUG_PRINT_SUPPORTED_CAMERA_SETTINGS
    TInt supportedFocusTypes = iCameraAdvancedSettings->SupportedAutoFocusTypes();

    qDebug() << "[CAMERA][FOCUS] Supported focus modes:";
    // First just output the supported modes
    if (supportedFocusTypes == CCamera::CCameraAdvancedSettings::EFocusModeUnknown)
        qDebug() << "[CAMERA][FOCUS] - EFocusModeUnknown";
    if (supportedFocusTypes & CCamera::CCameraAdvancedSettings::EFocusModeAuto)
        qDebug() << "[CAMERA][FOCUS] - EFocusModeAuto";
    if (supportedFocusTypes & CCamera::CCameraAdvancedSettings::EFocusModeFixed)
        qDebug() << "[CAMERA][FOCUS] - EFocusModeFixed";
    if (supportedFocusTypes & CCamera::CCameraAdvancedSettings::EFocusModeManual)
        qDebug() << "[CAMERA][FOCUS] - EFocusModeManual";
#endif

#ifdef DEBUG_PRINT_SUPPORTED_CAMERA_SETTINGS
    TInt supportedAutoFocusTypes = iCameraAdvancedSettings->SupportedAutoFocusTypes();

    qDebug() << "[CAMERA][AUTOFOCUS] Supported autofocus types:";
    // First just output the supported modes
    if (supportedAutoFocusTypes == CCamera::CCameraAdvancedSettings::EAutoFocusTypeOff)
        qDebug() << "[CAMERA][AUTOFOCUS] - EAutoFocusTypeOff";
    if (supportedAutoFocusTypes & CCamera::CCameraAdvancedSettings::EAutoFocusTypeContinuous)
        qDebug() << "[CAMERA][AUTOFOCUS] - EAutoFocusTypeContinuous";
    if (supportedAutoFocusTypes & CCamera::CCameraAdvancedSettings::EAutoFocusTypeSingle)
        qDebug() << "[CAMERA][AUTOFOCUS] - EAutoFocusTypeSingle";
#endif



#ifdef DEBUG_PRINT_SUPPORTED_CAMERA_SETTINGS
    TInt supportedFocusRanges = iCameraAdvancedSettings->SupportedFocusRanges();
    qDebug() << "[CAMERA][FOCUSRANGE] Supported focusrange modes:";
    // First just output the supported modes
    if (supportedFocusRanges == CCamera::CCameraAdvancedSettings::EFocusRangeAuto)
        qDebug() << "[CAMERA][FOCUSRANGE] - EFocusRangeAuto";
    if (supportedFocusRanges & CCamera::CCameraAdvancedSettings::EFocusRangeMacro)
        qDebug() << "[CAMERA][FOCUSRANGE] - EFocusRangeMacro";
    if (supportedFocusRanges & CCamera::CCameraAdvancedSettings::EFocusRangeNormal)
        qDebug() << "[CAMERA][FOCUSRANGE] - EFocusRangeNormal";
    if (supportedFocusRanges & CCamera::CCameraAdvancedSettings::EFocusRangeTele)
        qDebug() << "[CAMERA][FOCUSRANGE] - EFocusRangeTele";
    if (supportedFocusRanges & CCamera::CCameraAdvancedSettings::EFocusRangePortrait)
        qDebug() << "[CAMERA][FOCUSRANGE] - EFocusRangePortrait";
    if (supportedFocusRanges & CCamera::CCameraAdvancedSettings::EFocusRangeSuperMacro)
        qDebug() << "[CAMERA][FOCUSRANGE] - EFocusRangeSuperMacro";
    if (supportedFocusRanges & CCamera::CCameraAdvancedSettings::EFocusRangeHyperfocal)
        qDebug() << "[CAMERA][FOCUSRANGE] - EFocusRangeHyperfocal";
    if (supportedFocusRanges & CCamera::CCameraAdvancedSettings::EFocusRangeInfinite)
        qDebug() << "[CAMERA][FOCUSRANGE] - EFocusRangeInfinite";
#endif

#ifdef DEBUG_PRINT_SUPPORTED_CAMERA_SETTINGS
    TInt supportedAutoFocusAreas = iCameraAdvancedSettings->SupportedAutoFocusAreas();
    qDebug() << "[CAMERA][AUTOFOCUS] Supported autofocus areas:";
    // First just output the supported modes
    if (supportedAutoFocusAreas == CCamera::CCameraAdvancedSettings::EAutoFocusTypeAuto)
        qDebug() << "[CAMERA][AUTOFOCUS] - EAutoFocusTypeAuto";
    if (supportedAutoFocusAreas & CCamera::CCameraAdvancedSettings::EAutoFocusTypeSingleArea)
        qDebug() << "[CAMERA][AUTOFOCUS] - EAutoFocusTypeSingleArea";
    if (supportedAutoFocusAreas & CCamera::CCameraAdvancedSettings::EAutoFocusTypeMultiAreaCentered)
        qDebug() << "[CAMERA][AUTOFOCUS] - EAutoFocusTypeMultiAreaCentered";
#endif

#ifdef DEBUG_PRINT_SUPPORTED_CAMERA_SETTINGS
    TInt supportedWBModes = iCameraAdvancedSettings->SupportedWhiteBalanceModes();

    qDebug() << "[CAMERA][WB] Supported whitebalance modes:";
    // First just output the supported modes
    if (supportedWBModes == CCamera::EWBAuto)
        qDebug() << "[CAMERA][WB] - EWBAuto";
    if (supportedWBModes & CCamera::EWBDaylight)
        qDebug() << "[CAMERA][WB] - EWBDaylight";
    if (supportedWBModes & CCamera::EWBCloudy)
        qDebug() << "[CAMERA][WB] - EWBCloudy";
    if (supportedWBModes & CCamera::EWBTungsten)
        qDebug() << "[CAMERA][WB] - EWBTungsten";
    if (supportedWBModes & CCamera::EWBFluorescent)
        qDebug() << "[CAMERA][WB] - EWBFluorescent";
    if (supportedWBModes & CCamera::EWBFlash)
        qDebug() << "[CAMERA][WB] - EWBFlash";
    if (supportedWBModes & CCamera::EWBSnow)
        qDebug() << "[CAMERA][WB] - EWBSnow";
    if (supportedWBModes & CCamera::EWBBeach)
        qDebug() << "[CAMERA][WB] - EWBBeach";
    if (supportedWBModes & CCamera::EWBManual)
        qDebug() << "[CAMERA][WB] - EWBManual";
    if (supportedWBModes & CCamera::EWBShade)
        qDebug() << "[CAMERA][WB] - EWBShade";
    if (supportedWBModes & CCamera::EWBAutoSkin)
        qDebug() << "[CAMERA][WB] - EWBAutoSkin";
    if (supportedWBModes & CCamera::EWBHorizon)
        qDebug() << "[CAMERA][WB] - EWBHorizon";
    if (supportedWBModes & CCamera::EWBDaylightUnderWater)
        qDebug() << "[CAMERA][WB] - EWBDaylightUnderWater";
#endif


#ifdef DEBUG_PRINT_SUPPORTED_CAMERA_SETTINGS
    TInt supportedStabilModes = iCameraAdvancedSettings->SupportedStabilizationModes();
    qDebug() << "[CAMERA][STABIL] Supported stabilization modes:";
    // First just output the supported modes
    if (supportedStabilModes == CCamera::CCameraAdvancedSettings::EStabilizationModeOff)
        qDebug() << "[CAMERA][STABIL] - EStabilizationModeOff";
    if (supportedStabilModes & CCamera::CCameraAdvancedSettings::EStabilizationModeHorizontal)
        qDebug() << "[CAMERA][STABIL] - EStabilizationModeHorizontal";
    if (supportedStabilModes & CCamera::CCameraAdvancedSettings::EStabilizationModeVertical)
        qDebug() << "[CAMERA][STABIL] - EStabilizationModeVertical";
    if (supportedStabilModes & CCamera::CCameraAdvancedSettings::EStabilizationModeRotation)
        qDebug() << "[CAMERA][STABIL] - EStabilizationModeRotation";
    if (supportedStabilModes & CCamera::CCameraAdvancedSettings::EStabilizationModeAuto)
        qDebug() << "[CAMERA][STABIL] - EStabilizationModeAuto";
    if (supportedStabilModes & CCamera::CCameraAdvancedSettings::EStabilizationModeManual)
        qDebug() << "[CAMERA][STABIL] - EStabilizationModeManual";
#endif
#ifdef DEBUG_PRINT_SUPPORTED_CAMERA_SETTINGS
    TInt supportedStabilEffects = iCameraAdvancedSettings->SupportedStabilizationEffects();
    qDebug() << "[CAMERA][STABIL] Supported stabilization effects:";
    // First just output the supported modes
    if (supportedStabilEffects == CCamera::CCameraAdvancedSettings::EStabilizationOff)
        qDebug() << "[CAMERA][STABIL] - EStabilizationOff";
    if (supportedStabilEffects & CCamera::CCameraAdvancedSettings::EStabilizationAuto)
        qDebug() << "[CAMERA][STABIL] - EStabilizationAuto";
    if (supportedStabilEffects & CCamera::CCameraAdvancedSettings::EStabilizationFine)
        qDebug() << "[CAMERA][STABIL] - EStabilizationFine";
    if (supportedStabilEffects & CCamera::CCameraAdvancedSettings::EStabilizationMedium)
        qDebug() << "[CAMERA][STABIL] - EStabilizationMedium";
    if (supportedStabilEffects & CCamera::CCameraAdvancedSettings::EStabilizationStrong)
        qDebug() << "[CAMERA][STABIL] - EStabilizationStrong";
#endif

#ifdef DEBUG_PRINT_SUPPORTED_CAMERA_SETTINGS
    TInt supportedStabilComplexities = iCameraAdvancedSettings->SupportedStabilizationComplexityValues();
    qDebug() << "[CAMERA][STABIL] Supported stabilization complexities:";
    // First just output the supported modes
    if (supportedStabilComplexities == CCamera::CCameraAdvancedSettings::EStabilizationComplexityAuto)
        qDebug() << "[CAMERA][STABIL] - EStabilizationComplexityAuto";
    if (supportedStabilComplexities & CCamera::CCameraAdvancedSettings::EStabilizationComplexityLow)
        qDebug() << "[CAMERA][STABIL] - EStabilizationComplexityLow";
    if (supportedStabilComplexities & CCamera::CCameraAdvancedSettings::EStabilizationComplexityMedium)
        qDebug() << "[CAMERA][STABIL] - EStabilizationComplexityMedium";
    if (supportedStabilComplexities & CCamera::CCameraAdvancedSettings::EStabilizationComplexityHigh)
        qDebug() << "[CAMERA][STABIL] - EStabilizationComplexityHigh";
#endif

#ifdef DEBUG_PRINT_SUPPORTED_CAMERA_SETTINGS
    qDebug() << "[CAMERA] Min zoom factor: " << iCameraInfo.iMinZoomFactor;
    qDebug() << "[CAMERA] Max zoom factor: " << iCameraInfo.iMaxZoomFactor;
    qDebug() << "[CAMERA] Min zoom: " << iCameraInfo.iMinZoom;
    qDebug() << "[CAMERA] Max zoom: " << iCameraInfo.iMaxZoom;
    qDebug() << "[CAMERA] Max digital zoom: " << iCameraInfo.iMaxDigitalZoom;
    qDebug() << "[CAMERA] Max digital zoom factor: " << iCameraInfo.iMaxDigitalZoomFactor;
#endif

}
