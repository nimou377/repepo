#include "DecoderThread.h"

#include <QtCore/QDebug>

DecoderThread::DecoderThread(QObject *parent) :
    QThread(parent), m_type(BarReader::BarcodeType1D), m_queueMaxLength(3), m_stopped(false)
{
    qDebug() << "[DECODER] Created DecoderThread";
    m_reader = new BarReader(this);
}

DecoderThread::~DecoderThread()
{
    qDebug() << "[DECODER] Destroyed DecoderThread";
    m_stopped = true;
    quit();
    wait();
    delete m_reader;
}

void DecoderThread::stop()
{
    qDebug() << "[DECODER] DecoderThread::stop";
    m_stopped = true;
}

void DecoderThread::setDecodeCodeType(BarReader::BarCodeType type)
{
    qDebug() << "[DECODER] DecoderThread::setDecodeCodeType:" << type;
    m_type = type;
}

void DecoderThread::addFrameToDecodeQueue(QImage frame)
{
    if (m_queue.length() < m_queueMaxLength) {
        // Thread takes copy (wrapped has only one frame and it get's destroyed so we cannot store it)
        //qDebug() << "[DECODER] Added frame to decode queue, it has now" << m_queue.length() << "frames";
        QImage threadCopy = frame.copy();
        m_queue.enqueue(threadCopy);
        //qDebug() << "[DECODER] Got frame:" << threadCopy.size();
    } else {
        qDebug() << "[DECODER] Decode queue already has" << m_queueMaxLength << "frames, skipping";
    }
}

void DecoderThread::run()
{
    qDebug() << "[DECODER] DecoderThread::run";

    while (!m_stopped) {
        if (!m_queue.isEmpty()) {
            QImage currentFrame = m_queue.dequeue();
            QString code = m_reader->decode(currentFrame, m_type);
            if (!code.isEmpty()) {
                qDebug() << "[DECODER] Decoded string:" << code;
                emit stringDecoded(code);
            }
        } else {
            // Sleep for a while
            msleep(DECODER_THREAD_SLEEP_MS);
        }
    }
    qDebug() << "[DECODER] Thread stopped";
    exit(0);
}
