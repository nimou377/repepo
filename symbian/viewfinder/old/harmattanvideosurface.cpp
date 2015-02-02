#include "harmattanvideosurface.h"

const int VIDEO_FILLCOLOR = 0x000000;

HarmattanVideoSurface::HarmattanVideoSurface(QObject *parent) :
    QAbstractVideoSurface(parent)
{
    m_lastFrame.fill( VIDEO_FILLCOLOR );
}

HarmattanVideoSurface::~HarmattanVideoSurface()
{

}

bool HarmattanVideoSurface::start ( const QVideoSurfaceFormat & format )
{
    QAbstractVideoSurface::start( format );
    m_lastFrame.fill( VIDEO_FILLCOLOR );
    return true;
}

void HarmattanVideoSurface::stop()
{
    QAbstractVideoSurface::stop();
    m_lastFrame.fill( VIDEO_FILLCOLOR );
}

QList<QVideoFrame::PixelFormat> HarmattanVideoSurface::supportedPixelFormats(
        QAbstractVideoBuffer::HandleType handleType) const
{
    // Harmattan camera pipeline does not seem to support formats other than UYVY
    if (handleType == QAbstractVideoBuffer::NoHandle)
    {
        return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_UYVY;
    }
    else
    {
        return QList<QVideoFrame::PixelFormat>();
    }

}

bool HarmattanVideoSurface::present(const QVideoFrame &frame)
{
    QVideoFrame newFrame(frame);
    if( newFrame.map(QAbstractVideoBuffer::ReadOnly) )
    {

        m_lastFrame = toQImage( newFrame.width(), newFrame.height(), newFrame.mappedBytes(), newFrame.bits() );
        newFrame.unmap();
        emit frameAvailable();
        return true;
    }
    else
    {
        return false;
    }
}

QImage HarmattanVideoSurface::toQImage( int width, int height, int numBytes, const unsigned char* bytes )
{
    Q_UNUSED( numBytes );
    QImage image( width, height, QImage::Format_RGB32 );

    for( int y = 0; y < height; ++y )
    {
        const unsigned char* srcScanline = bytes + y*width*2;
        QRgb* destScanline = reinterpret_cast<QRgb*>( image.scanLine(y) );

        for( int x = 0; x < width; x+=2 )
        {
            const unsigned char* macroPixel = srcScanline + x*2;
            QRgb* imagePixel1 = destScanline + x;
            QRgb* imagePixel2 = destScanline + x + 1;

            int u = macroPixel[0];
            int y1 = macroPixel[1];
            int v = macroPixel[2];
            int y2 = macroPixel[3];

            *imagePixel1 = convertToRgb( u, y1, v );
            *imagePixel2 = convertToRgb( u, y2, v );

        }
    }

    return image;
}

QRgb HarmattanVideoSurface::convertToRgb( int u, int y, int v )
{
    int c = y - 16;
    int d = u - 128;
    int e = v - 128;

    return qRgb( qBound( 0, ( 298 * c           + 409 * e + 128 ) >> 8, 255 ),
                 qBound( 0, ( 298 * c - 100 * d - 208 * e + 128 ) >> 8, 255 ),
                 qBound( 0, ( 298 * c + 516 * d           + 128 ) >> 8, 255 ) );
}

