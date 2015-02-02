#include "videosurface.h"
#include <QVideoSurfaceFormat>
#include <QDebug>

const int FILLCOLOR = 0xFF0000;

VideoSurface::VideoSurface(QObject *parent) :
    QAbstractVideoSurface(parent)
{
    m_lastFrame.fill(FILLCOLOR);
}

VideoSurface::~VideoSurface()
{
    stop();
}

QImage VideoSurface::frame() const
{
    return m_lastFrame;
}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_ARGB32;
}

bool VideoSurface::present(const QVideoFrame &frame)
{
    if (frame.isValid())
    {
        QVideoFrame videoFrame(frame);
        if( videoFrame.map(QAbstractVideoBuffer::ReadOnly) )
        {
            m_lastFrame = QImage(videoFrame.width(), videoFrame.height(), QImage::Format_ARGB32);
            memcpy(m_lastFrame.bits(), videoFrame.bits(), videoFrame.mappedBytes());

            videoFrame.unmap();

            emit frameAvailable();
            return true;
        }
    }
    return false;
}

bool VideoSurface::start(const QVideoSurfaceFormat &format)
{
    if (isActive()) {
        stop();
    } else if (!format.frameSize().isEmpty()) {
        return QAbstractVideoSurface::start(format);
    }
    return false;
}

void VideoSurface::stop()
{
    m_lastFrame.fill(FILLCOLOR);
    QAbstractVideoSurface::stop();
}
