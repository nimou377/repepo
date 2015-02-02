#ifndef DECODERTHREAD_H
#define DECODERTHREAD_H

#include <QtCore/QThread>
#include <QtCore/QQueue>
#include <QtGui/QImage>
#include "BarReader.h"

static const int DECODER_THREAD_SLEEP_MS = 25;

class DecoderThread : public QThread
{
    Q_OBJECT
public:
    explicit DecoderThread(QObject *parent = 0);
    virtual ~DecoderThread();
signals:
    void stringDecoded(const QString& code);
public slots:
    void stop();
    void setDecodeCodeType(BarReader::BarCodeType type);
    void addFrameToDecodeQueue(QImage frame);
protected:
    virtual void run();
private:
    BarReader* m_reader;
    BarReader::BarCodeType m_type;
    QQueue<QImage> m_queue;
    int m_queueMaxLength;
    bool m_stopped;

};

#endif // DECODERTHREAD_H
