/**
 * Copyright (c) 2012 Harald Meyer, http://www.meh.at
 */

#include "fipthread.h"

#include <QDebug>
#include <QDesktopServices>
#include <QDateTime>
#include "blackandwhiteeffect.h"

FIPThread::FIPThread(QObject *parent) :
    QThread(parent),
    m_currentMode(EMode_Live),
    m_frameIdx(-1),
    m_stateProcessing(false),
    m_abort(false),
    m_restart(false),
    m_effectThreshold(50)
{
}

FIPThread::~FIPThread()
{
    // Wait for the worker thread to finish.
    m_mutex.lock();
    m_abort = true;
    m_condition.wakeOne();
    m_mutex.unlock();

    wait();
}

/*!
  Get latest processed image. The image is locked to prevent overriding.
  getLatestProcessedImageReady() has to be called to unlock.
  If no processed image is available then NULL is returned.
*/
QImage * FIPThread::getLatestProcessedImage()
{
    m_mutex.lock();
    if (m_frameIdx == -1 || m_frames[m_frameIdx].isNull())
    {
        m_mutex.unlock();
        return NULL;
    }

    return &m_frames[m_frameIdx];
}

/*!
  Unlock after getting the processed image.
*/
void FIPThread::getLatestProcessedImageReady()
{
    m_mutex.unlock();
}

/*!
  Set a new live video frame. The contents of the frame are copied to a local buffer.
*/
void FIPThread::setNewFrame(QVideoFrame *ptrFrame)
{
    // Drop frame if last frame is still being processed or not in live mode
    if (m_stateProcessing || m_currentMode != EMode_Live)
        return;

    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker); // hide warning in Qt Creator

    // Select buffer which is not in use at the moment
    if (m_frameIdx < 0) m_frameIdx = 0;

    int bufferIdx = 1 - m_frameIdx;

    if (m_frames[bufferIdx].isNull() || m_frames[bufferIdx].width() != ptrFrame->width() ||
        m_frames[bufferIdx].height() != ptrFrame->height()) {
        m_frames[bufferIdx] = QImage(ptrFrame->width(), ptrFrame->height(), QImage::Format_ARGB32);
    }

    // Copy data to local buffer
    memcpy(m_frames[bufferIdx].bits(), ptrFrame->bits(), ptrFrame->mappedBytes());

    // Start processing
    m_abort = false;
    if (!isRunning()) {
        start(LowPriority);
    } else {
        m_restart = true;
        m_condition.wakeOne();
    }
}

/*!
  Set full resolution frame (containing JPEG EXIF data).
  The data is copied to a buffer.
*/
void FIPThread::setFullResolutionFrame(QVideoFrame *ptrFrame)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker); // hide warning in Qt Creator

    // Decode and copy frame data to local buffer.
    // "loadFromData()" consumes a lot of time. To improve performance, the raw data should be copied
    // and "loadFromData()" be called in "run()".
    if (m_fullResFrame.loadFromData(ptrFrame->bits(), ptrFrame->mappedBytes()))
    {
        m_currentMode = EMode_Captured;

        // Start processing
        m_abort = false;
        if (!isRunning()) {
            start(LowPriority);
        } else {
            m_restart = true;
            m_condition.wakeOne();
        }
    }
}

/*!
  Set the black&white effect's threshold.
*/
void FIPThread::setEffectThreshold(const int &thresh)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker); // hide warning in Qt Creator
    m_effectThreshold = thresh;
}

/*!
  The QThread working loop. Set m_abort=true to leave.
*/
void FIPThread::run()
{
    forever
    {
        int effectThreshold;
        TMode currentMode;
        BlackAndWhiteEffect effect;
        int curIdx;
        QImage *ptrImage;

        // We "freeze" the state by copying class variables to local variables.
        m_mutex.lock();
        m_stateProcessing = true;
        effectThreshold = m_effectThreshold;
        currentMode = m_currentMode;
        m_mutex.unlock();

        // In live mode we use double buffering
        if (currentMode == EMode_Live)
        {
            curIdx = 1 - m_frameIdx;
            ptrImage = &m_frames[curIdx];
        }
        else
        {
            curIdx = m_frameIdx;

            // Convert to the right format (ARGB32) for sake of simplicity
            if (m_fullResFrame.format() != QImage::Format_ARGB32)
                m_fullResFrame = m_fullResFrame.convertToFormat(QImage::Format_ARGB32);

            ptrImage = &m_fullResFrame;
        }

        // Apply effect directly to the source image (overriding it).
        effect.applyEffect(*ptrImage, *ptrImage, effectThreshold);

        if (currentMode == EMode_Captured)
        {
            // Save image
            QString fn = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation) +
                    QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss.jpg");
            if (!ptrImage->save(fn))
                qDebug() << "Error while writing to " << fn;
            else
                emit fullImageSaved(fn);

            // Free memory of full-resolution buffer
            m_fullResFrame = QImage();
        }
        else
        {
            // Signal that a new processed frame is available.
            // There is no guarantee that *this* frame is available with "getLatestProcessedImage()".
            // For this scenario the latest frame is sufficient.
            emit newFrameReady();
        }

        // Now we are ready for the next frame.
        m_mutex.lock();
        m_frameIdx = curIdx;
        m_stateProcessing = false;

        if (m_abort)
        {
            m_mutex.unlock();
            return;
        }
        if (!m_restart)
        {
            // Block the loop and wait for new data
            m_condition.wait(&m_mutex);
        }
        m_restart = false;
        m_mutex.unlock();
    }
}



