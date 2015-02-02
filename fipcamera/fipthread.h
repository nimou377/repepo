/**
 * Copyright (c) 2012 Harald Meyer, http://www.meh.at
 */

#ifndef FIPTHREAD_H
#define FIPTHREAD_H

#include <QThread>
#include <QImage>
#include <QVideoFrame>
#include <QMutex>
#include <QWaitCondition>


class FIPThread : public QThread
{
    Q_OBJECT
public:
    explicit FIPThread(QObject *parent = 0);
    virtual ~FIPThread();

    void setNewFrame(QVideoFrame *ptrFrame);
    void setFullResolutionFrame(QVideoFrame *ptrFrame);

    // pull latest processed image. Returns NULL if no image is available.
    QImage * getLatestProcessedImage();
    void getLatestProcessedImageReady();

    // Worker loop
    void run();

    // Is an image currently processed?
    inline bool isProcessing() const {
        return m_stateProcessing;
    }

    void setEffectThreshold(const int &thresh);

    inline int getEffectThreshold() const {
        return m_effectThreshold;
    }

    inline void setLiveMode() {
        m_currentMode = EMode_Live;
    }
    
Q_SIGNALS:
    void newFrameReady();
    void fullImageSaved(QString fn);
    
public Q_SLOTS:

private:
    enum TMode {
        EMode_Live,
        EMode_Captured
    };

    TMode m_currentMode;

    int m_frameIdx; // current buffer marked as ready
    QImage m_frames[2]; // double buffer
    QImage m_fullResFrame;

    bool m_stateProcessing;

    QMutex m_mutex;
    QWaitCondition m_condition;

    bool m_abort;
    bool m_restart;

    int m_effectThreshold;
};

#endif // FIPTHREAD_H
