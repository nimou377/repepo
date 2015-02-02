#ifndef ViewFinderWrapper_H
#define ViewFinderWrapper_H

#include <QObject>
#include <QFrame>
#include <QRect>
#include <QVector>
#include <QPoint>
#include <QDeclarativeItem>
#include "BarReader.h"

#include "ViewFinderCameraInterface.h"

static const int LINE_WIDTH = 2;
static const int DECODE_EVERY_NTH_FRAME = 25;

class QPixmap;
class QPainter;
class DecoderThread;

class ViewFinderWrapper : public QDeclarativeItem, public ViewFinderCameraObserver
{
    Q_OBJECT
    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(QColor overlayColor READ overlayColor WRITE setOverlayColor NOTIFY overlayColorChanged)
    Q_PROPERTY(int decodeInterval READ decodeFrameInterval WRITE setDecodeFrameInterval NOTIFY decodeIntervalChanged)
    Q_PROPERTY(bool hasAutoFocus READ hasAutoFocus NOTIFY autoFocusChanged)
public:
    enum ViewFinderMode { ViewFinderModeNotSet, ViewFinderMode1DCode, ViewFinderModeDM, ViewFinderModeQR };

public:
    explicit ViewFinderWrapper(QDeclarativeItem *parent = 0);
    virtual ~ViewFinderWrapper();

signals:
    void modeChanged();
    void lineWidthChanged();
    void overlayColorChanged();

    // TODO camera signals
    void cameraReady();
    void cameraFailed(int error = 0);

    void viewFinderStarted();
    void viewFinderStopped(int error = 0);

    void decodeIntervalChanged();
    void stringDecoded(const QString& code);

    void autoFocusChanged();
public slots:
    int mode() const;
    void setMode(int mode);

    int lineWidth() const;
    void setLineWidth(int width);

    QColor overlayColor() const;
    void setOverlayColor(const QColor& color);

    void startViewFinder();
    void stopViewFinder();

    void startDecoding();
    void stopDecoding();

    int decodeFrameInterval() const;
    void setDecodeFrameInterval(const int interval);

    bool hasAutoFocus();
private slots:
    void onStringDecoded(const QString& code);
private:
    void openCamera();
    void closeCamera();
    void setDecoderThreadMode(int mode);
private: // From ViewFinderCameraObserver
    virtual void onCameraError(int error);
    virtual void onCameraReservationSucceed();
    virtual void onCameraReservationFailed(int error);
    virtual void onViewFinderStartFailed(int error);
    virtual void onViewFinderAlreadyRunning();
    virtual void onViewFinderStarted();
    virtual void onViewFinderStopped();
    virtual void onViewFinderFrameReady();
private: // From QDeclarativeItem
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void draw1DViewFinder(QPainter &painter, QRect rect);
    void drawDMViewFinder(QPainter &painter, QRect rect);
    void drawQRViewFinder(QPainter &painter, QRect rect);
    void drawOverlayPrepare(QPainter &painter);
    QVector<QPoint> getOuterViewFinderPoints(QRect rect, bool addQrCodeLines=false);
    qreal getOpacity() const;

private: // Data
    ViewFinderCamera* m_camera;
    DecoderThread* m_decoder;
    QPixmap m_frame;
    int m_mode;
    int m_lineWidth;
    QColor m_overlayColor;
    QSize m_wantedViewfinderSize;
    bool m_drawOverlay;
    bool m_codeFound;

    long m_frameCounter;
    int m_decodeEveryNthFrame;
    mutable double m_opacityCounter;
};

#endif // ViewFinderWrapper_H
