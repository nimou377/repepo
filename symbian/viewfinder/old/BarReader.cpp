#include "BarReader.h"

#ifdef HAVE_1D_READER
#include "1dreader.h"
#endif

#ifdef HAVE_ZXING
#include "Zxing.h"
#endif

#ifdef HAVE_LIBDMTX
#include "DataMatrix.h"
#endif

#include <QtCore/QDebug>

BarReader::BarReader(QObject* parent) : QObject(parent)
{
    qDebug() << "[CODE] BarReader::BarReader";
#ifdef HAVE_1D_READER
    qDebug() << "[CODE] COMPILED IN: 1D Reader";
#endif
#ifdef HAVE_ZXING
    qDebug() << "[CODE] COMPILED IN: ZXing";
#endif
#ifdef HAVE_LIBDMTX
    qDebug() << "[CODE] COMPILED IN: libdmtx";
#endif
#if !(defined(HAVE_1D_READER) || defined (HAVE_ZXING) || defined (HAVE_LIBDMTX))
    qDebug() << "[CODE] NO BARCODE READING LIBS COMPILED IN!!! ";
    qDebug() << "[CODE] NOTHING CAN BE RECOGNIZED!!!";
#endif
}

BarReader::~BarReader()
{
    qDebug() << "[CODE] BarReader::~BarReader";
}

QString BarReader::decode( const QImage& frame,  BarCodeType type )
{
    QString result("");

    if (type == BarcodeTypeNotSet) {
        return result;
    }

    switch (type) {
    case BarcodeType1D:
        // Try default proprietary 1D-reader
        result = decode1D(frame);
        break;
    case BarcodeTypeQR:
        result = decodeQR(frame);
        break;
    case BarcodeTypeDataMatrix:
        // libdmtx
        result = result = decodeDataMatrix(frame);
        break;
    default:
        break;
    }

    if (!result.isEmpty()) {
        qDebug() << "[CODE] Result:" << result;
    }
    return result;
}


QString BarReader::decode1D( const QImage& frame )
{
    QString result;
#ifdef HAVE_1D_READER
    // First we try with our old proprietary 1D-decoder
    int scanlineIndex = frame.height() / 2;
    const uchar* scanlineData = frame.constScanLine(scanlineIndex);
    OneDReader::ScanLine scanline( frame.width());
    scanline.scan( reinterpret_cast<const quint32*>(scanlineData) );

    OneDReader::CodeValue value;
    OneDReader::Analyzer analyzer( scanline );

    if( analyzer.decode(OneDReader::TYPE_ALL, value ) )
    {
        result = value.code();
    }
    if (!result.isEmpty()) {
        qDebug() << "[CODE] 1D-code recognized by proprietary reader";
    }
#endif
#ifdef HAVE_ZXING
    // Then if no result, try ZXing
    if (result.isEmpty()) {
        result = ZXing::decode(ZXing::OneD, frame);
        if (!result.isEmpty()) {
            qDebug() << "[CODE] 1D-code recognized by ZXing 1D-reader";
        }
    }
#endif
    return result;
}

QString BarReader::decodeQR(const QImage& frame)
{
    QString result("");
#ifdef HAVE_ZXING
    // 1st try ZXing QRCode
    result = ZXing::decode(ZXing::QrCode, frame);
    if (!result.isEmpty()) {
        qDebug() << "[CODE] 2D-code recognized by ZXing QRCode-reader";
    }
//    if( result.isEmpty() ) {
//        // 2nd try ZXing Databar
//        result =  ZXing::decode(ZXing::DataBar, frame);
//    }
//    if( result.isEmpty() ) {
//        // 3rd try ZXing DataMatrix
//        result =  ZXing::decode(ZXing::DataMatrix, frame);
//    }
#endif
    return result;
}

QString BarReader::decodeDataMatrix(const QImage& frame)
{
    QString result;
//#ifdef HAVE_ZXING
//    return ZXing::decode(ZXing::DataMatrix, frame);
//#else
//    qDebug() << "ZXing not compiled in";
//#endif
#ifdef HAVE_LIBDMTX
    // Slow
    result = DataMatrix::decode(frame);
    if (!result.isEmpty()) {
        qDebug() << "[CODE] 2D-code recognized by libdmtx-reader";
    }
#endif
    return result;
}
