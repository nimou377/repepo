#ifndef HARMATTANVIDEOSURFACE_H
#define HARMATTANVIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <QImage>

class HarmattanVideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit HarmattanVideoSurface(QObject *parent = 0);
    virtual ~HarmattanVideoSurface();
signals:
    void frameAvailable();
public:
    virtual bool start ( const QVideoSurfaceFormat & format );
    virtual void stop();

    virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
    virtual bool present(const QVideoFrame &frame);
private:
    QImage toQImage( int width, int height, int numBytes, const unsigned char* bytes );
    inline QRgb convertToRgb( int u, int y, int v );

private:
    QImage m_lastFrame;
};

#endif // HARMATTANVIDEOSURFACE_H
