#include "ViewFinderCameraSimulator.h"
#include <QtCore/QDebug>

ViewFinderCameraSimulator::ViewFinderCameraSimulator(ViewFinderCameraObserver& observer, QObject *parent) :
    QObject(parent), m_observer(observer), m_size(360, 270), m_frameCounter(0), m_testImageIndex(0)
{
    qDebug() << "ViewFinderCameraSimulator::ViewFinderCameraSimulator";

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));

    //m_frame = new QPixmap(":/test/0001.png");
    m_frame = new QPixmap(m_size.width(), m_size.height());
    m_frame->fill(Qt::green);

    QTimer::singleShot(1000, this, SLOT(initTimeout()));
}

ViewFinderCameraSimulator::~ViewFinderCameraSimulator()
{
    qDebug() << "ViewFinderCameraSimulator::~ViewFinderCameraSimulator";
    if (m_frame) {
        delete m_frame;
    }
}

void ViewFinderCameraSimulator::StartViewFinder(QSize size)
{
    qDebug() << "ViewFinderCameraSimulator::StartViewFinder";

    m_size = size;

    const int SIMULATOR_FRAME_INTERVAL = 1000/SIMULATOR_VIEWFINDER_FPS;
    m_timer.setInterval(SIMULATOR_FRAME_INTERVAL);
    m_timer.start();
    m_observer.onViewFinderStarted();
}

void ViewFinderCameraSimulator::StopViewFinder()
{
    qDebug() << "ViewFinderCameraSimulator::StopViewFinder";
    m_timer.stop();
    m_observer.onViewFinderStopped();
}

QPixmap* ViewFinderCameraSimulator::Frame()
{
    return m_frame;
}

void ViewFinderCameraSimulator::initTimeout()
{
    qDebug() << "ViewFinderCameraSimulator::initTimeout";
    m_observer.onCameraReservationSucceed();
}

void ViewFinderCameraSimulator::timeout()
{
    // To simulate changing scenery, load something to m_frame here
    ++m_frameCounter;
    if (m_frameCounter > 100) {
        m_frameCounter = 0;
        nextTestImage();
    }
    m_observer.onViewFinderFrameReady();
}

void ViewFinderCameraSimulator::nextTestImage()
{
    qDebug() << "ViewFinderCameraSimulator::nextTestImage: " << m_testImageIndex;
//    const int TEST_IMAGE_COUNT = 9;
//    ++m_testImageIndex;
//    if (m_testImageIndex > TEST_IMAGE_COUNT) {
//        m_testImageIndex = 0;
//    }
//    QString TEST_IMAGE_NAME("qrc:/test/000%1.png");
//    QString test_image_name = TEST_IMAGE_NAME.arg(m_testImageIndex);
//    qDebug() << "Change test image to" << test_image_name;
//    QPixmap* pixmap = new QPixmap(test_image_name);
    QPixmap* pixmap = new QPixmap(m_size.width(), m_size.height());
    pixmap->fill(Qt::green);
    if (m_frame) {
        delete m_frame;
        m_frame = 0;
    }
    m_frame = pixmap;
}
