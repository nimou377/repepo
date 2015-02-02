#ifndef BARREADER_H
#define BARREADER_H

#include <QtCore/QObject>
#include <QtGui/QPixmap>

class BarReader : public QObject
{
    Q_OBJECT
public:
    enum BarCodeType {
        BarcodeTypeNotSet = 0,
        BarcodeType1D,
        BarcodeTypeQR,
        BarcodeTypeDataMatrix
    };

    explicit BarReader(QObject* parent = 0);
    virtual ~BarReader();
signals:
    void stringDecoded(const QString& code);
public slots:
    QString decode( const QImage& frame, BarCodeType type = BarcodeTypeNotSet );
private:
    QString decode1D( const QImage& frame );
    QString decodeQR(const QImage& frame);
    QString decodeDataMatrix(const QImage& frame);

};

#endif // BARREADER_H
