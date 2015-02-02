#ifndef PROCESSINGTHREAD_H
#define PROCESSINGTHREAD_H

#include <QThread>
#include <QImage>
#include <QQueue>

class ProcessingThread : public QThread
{
    Q_OBJECT
public:
    explicit ProcessingThread(QObject *parent = 0);
    virtual ~ProcessingThread();
signals:
    void frameProcessed();
    void queueFull();
public:
    void stop();
    void addFrameToProcessingQueue(QImage frame);
private:
    virtual void run();
private:
    QQueue<QImage> m_queue;
    int m_queueMaxLength;
    bool m_stopped;
};

#endif // PROCESSINGTHREAD_H
