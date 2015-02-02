#include "processingthread.h"
#include <QDebug>

static const int QUEUE_MAX_LENGTH = 3;
static const int THREAD_SLEEP_MS = 25;

ProcessingThread::ProcessingThread(QObject *parent) :
    QThread(parent), m_stopped(false), m_queueMaxLength(QUEUE_MAX_LENGTH)
{
}

ProcessingThread::~ProcessingThread()
{
    stop();
}

void ProcessingThread::stop()
{
    m_stopped = true;
}

void ProcessingThread::addFrameToProcessingQueue(QImage frame)
{
    if (m_queue.length() < m_queueMaxLength) {
        QImage threadCopy = frame.copy();
        m_queue.enqueue(threadCopy);
    } else {
        emit queueFull();
    }
}

void ProcessingThread::run()
{
    // Process until stop() called
    while (!m_stopped)
    {
        if (!m_queue.isEmpty())
        {
            QImage currentFrame = m_queue.dequeue();

            // Here you can do whatever processing you need on the frame, like detect barcodes, etc.

            emit frameProcessed();
        }
        else
        {
            // No frames in queue, sleep for a short while
            msleep(THREAD_SLEEP_MS);
        }
    }
    qDebug() << "Processing thread ending";
    exit(0);
}
