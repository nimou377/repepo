#include <QApplication>
#include <QPixmap>
#include <QDebug>
#include <QPainter>
#include <QLabel>
#include <QHBoxLayout>
#include <QStyleOptionGraphicsItem>
#include <QTimer>
#include <cmath>

#include "ViewFinderWrapper.h"
#include "DecoderThread.h"

#if defined(Q_OS_SYMBIAN)
#include "ViewFinderCameraSymbian.h"
#elif defined(Q_OS_MEEGO)
#include "ViewFinderCameraHarmattan.h"
#else
#include "ViewFinderCameraSimulator.h"
#endif


ViewFinderWrapper::ViewFinderWrapper(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_decoder(0),
      m_frame(),
      m_mode(ViewFinderModeNotSet),
      m_lineWidth(LINE_WIDTH),
      m_overlayColor(Qt::red),
      m_wantedViewfinderSize(320,240),
      m_drawOverlay(false),
      m_codeFound(false),
      m_frameCounter(0),
      m_decodeEveryNthFrame(DECODE_EVERY_NTH_FRAME),
      m_opacityCounter(0)
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::ViewFinderWrapper";
    setFlags(QGraphicsItem::ItemIsFocusable);

    // Camera is opened in ctor and closed in dtor. Viewfinder is controlled separately
    openCamera();
    m_decoder = new DecoderThread(this);
    connect(m_decoder, SIGNAL(stringDecoded(QString)), this, SLOT(onStringDecoded(QString)));
}

ViewFinderWrapper::~ViewFinderWrapper()
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::~ViewFinderWrapper";

    stopDecoding();

    m_decoder->deleteLater();
    m_decoder = 0;

    stopViewFinder();

    closeCamera();
}

int ViewFinderWrapper::mode() const
{
    return m_mode;
}

void ViewFinderWrapper::setMode(int mode)
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::setMode:" << mode;
    if (mode != m_mode) {
        m_mode = mode;
        setDecoderThreadMode(m_mode);
        emit modeChanged();
        update();
    }
}

int ViewFinderWrapper::lineWidth() const
{
    return m_lineWidth;
}

void ViewFinderWrapper::setLineWidth(int width)
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::setLineWidth:" << width;
    if (width != m_lineWidth) {
        m_lineWidth = width;
        emit lineWidthChanged();
        update();
    }
}

QColor ViewFinderWrapper::overlayColor() const
{
    return m_overlayColor;
}

void ViewFinderWrapper::setOverlayColor(const QColor& color)
{
    if (color != m_overlayColor) {
        m_overlayColor = color;
        emit overlayColorChanged();
    }
}

void ViewFinderWrapper::startViewFinder()
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::startViewFinder: " << width() << "x" << height();
    m_wantedViewfinderSize = QSize(width(), height());
    qDebug() << "[WRAPPER] m_wantedViewfinderSize:" << m_wantedViewfinderSize;
    m_codeFound = false;
    if (m_camera) {
        //m_camera->StartViewFinder(QSize(width(),height()));
        m_camera->StartViewFinder(QSize(1024, 768));
    }
}

void ViewFinderWrapper::stopViewFinder()
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::stopViewFinder";
    if (m_camera) {
        m_camera->StopViewFinder();
    }
}

void ViewFinderWrapper::startDecoding()
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::startDecoding";
    if (!m_decoder->isRunning()) {
        m_decoder->start();
    } else {
        qDebug() << "[WRAPPER] startDecoding called even though thread is already running";
    }
}

void ViewFinderWrapper::stopDecoding()
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::stopDecoding";
    if (!m_decoder) {
        qDebug() << "[WRAPPER][ERROR] There is no DecoderThread!";
        return;
    }
    m_decoder->stop();
    disconnect(m_decoder, 0, 0, 0);
}

int ViewFinderWrapper::decodeFrameInterval() const
{
    return m_decodeEveryNthFrame;
}

void ViewFinderWrapper::setDecodeFrameInterval(const int interval)
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::setDecodeFrameInterval:" << interval;
    if (interval != m_decodeEveryNthFrame) {
        m_decodeEveryNthFrame = interval;
        emit decodeIntervalChanged();
    }
}

bool ViewFinderWrapper::hasAutoFocus()
{
    bool hasAutoFocus = m_camera->hasAutoFocus();

    qDebug() << "[WRAPPER] hasAutoFocus: " << hasAutoFocus;
    return hasAutoFocus;
}

void ViewFinderWrapper::onStringDecoded(const QString& code)
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::onStringDecoded:" << code;
    m_codeFound = true;
    emit stringDecoded(code);
}

void ViewFinderWrapper::openCamera()
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::openCamera";
#if defined(Q_OS_SYMBIAN)
    qDebug() << "[WRAPPER] Symbian camera";
    TRAPD( err, m_camera = CViewFinderCamera::NewL(*this); );
#elif defined(Q_OS_MEEGO)
    qDebug() << "[WRAPPER] Harmattan camera";
    m_camera = new ViewFinderCameraHarmattan(*this);
#else
    qDebug() << "[WRAPPER] Simulator camera";
    m_camera = new ViewFinderCameraSimulator(*this);
#endif
}

void ViewFinderWrapper::closeCamera()
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::closeCamera";

    // Delete camera
    if (m_camera) {
        m_camera->Cleanup();
        delete m_camera;
        m_camera = 0;
    }
}

void ViewFinderWrapper::setDecoderThreadMode(int mode)
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::setDecoderThreadMode:" << mode;
    m_codeFound = false;
    if (m_decoder) {
        switch (mode) {
        case ViewFinderMode1DCode:
            m_decoder->setDecodeCodeType(BarReader::BarcodeType1D);
            break;
        case ViewFinderModeDM:
            m_decoder->setDecodeCodeType(BarReader::BarcodeTypeDataMatrix);
            break;
        case ViewFinderModeQR:
            m_decoder->setDecodeCodeType(BarReader::BarcodeTypeQR);
            break;
        case ViewFinderModeNotSet:
        default:
            m_decoder->setDecodeCodeType(BarReader::BarcodeTypeNotSet);
            break;
        }
    } else {
        qDebug() << "[WRAPPER] Not decoding yet";
    }
}

void ViewFinderWrapper::onCameraError(int error)
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::cameraError:" << error;
}

void ViewFinderWrapper::onCameraReservationSucceed()
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::cameraReservationSucceeded";
    emit cameraReady();
}

void ViewFinderWrapper::onCameraReservationFailed(int error)
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::cameraReservationFailed:" << error;
    emit cameraFailed(error);
}

void ViewFinderWrapper::onViewFinderStartFailed(int error)
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::viewFinderPrepareFailed: " << error;
    emit viewFinderStopped(error);
}

void ViewFinderWrapper::onViewFinderAlreadyRunning()
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::viewFinderAlreadyRunning";
}

void ViewFinderWrapper::onViewFinderStarted()
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::viewFinderStarted";
    m_drawOverlay = true;
    emit viewFinderStarted();
}

void ViewFinderWrapper::onViewFinderStopped()
{
    qDebug() << "[WRAPPER] ViewFinderWrapper::viewFinderStopped";
    emit viewFinderStopped();
}

void ViewFinderWrapper::onViewFinderFrameReady()
{
    ++m_frameCounter;
    QPixmap* newFrame = m_camera->Frame();
    if (newFrame) {
        m_frame = newFrame->copy();
    }
    update();

    // If we have a decoder, send data for decoding...
    if (m_decoder) {
        if (m_frameCounter > m_decodeEveryNthFrame) {
            QImage image = m_frame.toImage();
            m_decoder->addFrameToDecodeQueue(image);
            m_frameCounter = 0;
        }
    }
}

void ViewFinderWrapper::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRect rect = option->rect;

    if (!m_frame.isNull()) {
        // frame is only scaled down for drawing, decoding from the original
        QPixmap frame = m_frame.scaled(rect.size(), Qt::KeepAspectRatio);
        if (!frame.isNull()) {
            painter->drawPixmap(rect, frame);
        }
    }

    if (m_drawOverlay) {
        if (mode() == ViewFinderMode1DCode) {
            draw1DViewFinder(*painter, rect);
        } else if (mode() == ViewFinderModeDM) {
            drawDMViewFinder(*painter, rect);
        } else {
            drawQRViewFinder(*painter, rect);
        }
    }
}

void ViewFinderWrapper::draw1DViewFinder(QPainter &painter, QRect rect)
{
    const int margin = 10;
    const int width = rect.width() - 2*margin;
    const int height = rect.height();
    QLineF line(margin, rect.y() + height/2, width, rect.y() + height/2);

    drawOverlayPrepare(painter);
    painter.drawLine(line);
}

void ViewFinderWrapper::drawDMViewFinder(QPainter &painter, QRect rect)
{
    drawOverlayPrepare(painter);
    QVector<QPoint> points = getOuterViewFinderPoints(rect);
    painter.drawLines(points);
}

void ViewFinderWrapper::drawQRViewFinder(QPainter &painter, QRect rect)
{
    drawOverlayPrepare(painter);
    QVector<QPoint> points = getOuterViewFinderPoints(rect, true);
    painter.drawLines(points);
}

void ViewFinderWrapper::drawOverlayPrepare(QPainter &painter)
{
    QPen pen;
    if (m_codeFound) {
        pen.setColor(Qt::green);
    } else {
        pen.setColor(m_overlayColor);
    }
    pen.setWidthF(m_lineWidth);
    pen.setCapStyle(Qt::RoundCap);
    qreal opacity = getOpacity();
    painter.setOpacity(opacity);
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setPen(pen);

}

QVector<QPoint> ViewFinderWrapper::getOuterViewFinderPoints(QRect rect, bool addQrCodeLines)
{
    const int width = rect.width();
    const int height = rect.height();
    const QPoint tl(rect.x() + width/6, rect.y() + height/6);
    const QPoint tr(rect.x() + width - width/6, rect.y() + height/6);
    const QPoint bl(rect.x() + width/6, rect.y() + height - height/6);
    const QPoint br(rect.x() + width - width/6, rect.y() + height - height/6);
    const int areaLen = width / 4;
    const int qrAreaLen = areaLen / 2;

    QVector<QPoint> points;

    // Top left
    points << tl + QPoint(0, areaLen);
    points << tl;

    points << tl;
    points << tl + QPoint(areaLen, 0);

    if (addQrCodeLines) {
        points << tl + QPoint(0, qrAreaLen);
        points << tl + QPoint(qrAreaLen, qrAreaLen);

        points << tl + QPoint(qrAreaLen, qrAreaLen);
        points << tl + QPoint(qrAreaLen, 0);
    }

    // Top Right
    points << tr + QPoint(-areaLen, 0);
    points << tr;

    points << tr;
    points << tr + QPoint(0, areaLen);

    if (addQrCodeLines) {
        points << tr + QPoint(-qrAreaLen, 0);
        points << tr + QPoint(-qrAreaLen, qrAreaLen);

        points << tr + QPoint(-qrAreaLen, qrAreaLen);
        points << tr + QPoint(0, qrAreaLen);
    }


    // Bottom left
    points << bl + QPoint(0, -areaLen);
    points << bl;

    points << bl;
    points << bl + QPoint(areaLen, 0);

    if (addQrCodeLines) {
        points << bl + QPoint(0, -qrAreaLen);
        points << bl + QPoint(qrAreaLen, -qrAreaLen);

        points << bl + QPoint(qrAreaLen, -qrAreaLen);
        points << bl + QPoint(qrAreaLen, 0);
    }

    // Bottom right
    points << br + QPoint(-areaLen, 0);
    points << br;

    points << br;
    points << br + QPoint(0, -areaLen);

    return points;
}

qreal ViewFinderWrapper::getOpacity() const
{
    m_opacityCounter += M_PI/30;
    if (m_opacityCounter > 2*M_PI) {
        m_opacityCounter = 0;
    }

    qreal result = sin(m_opacityCounter);
    if (result < 0)
    {
        result = -result;
    }

    return result;
}
