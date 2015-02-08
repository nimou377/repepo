

#include <QtWidgets>

#include <QDebug>

#include "mainWindow.h"
#include "mylabel.h"


MainWindow::MainWindow()
{
    //----------
    b0=false;b1=false;
    i0=0;i1=0;
    //----------
    imageLabel = new MyLabel(this);
    //    imageLabel->setBackgroundRole(QPalette::Base);
    //    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    setCentralWidget(imageLabel);

    connect(imageLabel,SIGNAL(clicked(int,int)),this,SLOT(mySlot(int,int)));

    //resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
    qim.load(":/res/Lenna.png","PNG");
    imageLabel->setPixmap(QPixmap::fromImage(qim));
#ifdef Q_OS_WIN
    imageLabel->setFixedSize(700,700);
#endif
#ifdef Q_OS_ANDROID
    imageLabel->setFixedSize(1000,1000);
#endif


    for ( int i = 0; i < 256; ++i )
        sColorTable.push_back( qRgb( i, i, i ) );

}
void MainWindow::mySlot(int x, int y){
    switch(i0){
    case 0:
        qDebug() << "case 0:";
        qDebug() << qim.format();
        mat = cv::Mat(qim.height(), qim.width(), CV_8UC4,
                      const_cast<unsigned char*>(qim.bits()),
                      qim.bytesPerLine());
        if(! mat.data ){
            qDebug() <<  "mat has no data" ;
        }
        //return (inCloneImageData ? mat.clone() : mat);
        break;
    case 1:
        qDebug() << "case 1:";
        cvtColor( mat, grayMat, CV_BGR2GRAY );
        mat = grayMat;
        qDebug() << "grayMat type: " << grayMat.type();
        break;
    case 2:
        qDebug() << "case 2:";
        switch(mat.type()){
        case CV_8UC1:
            qDebug() << "case CV_8UC1:";
            newQim = QImage( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8 );
            newQim.setColorTable( sColorTable );
            break;
        case CV_8UC3:
            qDebug() << "case CV_8UC3:";
            newQim = QImage( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888 );
            break;
        case CV_8UC4:
            qDebug() << "case CV_8UC4:";
            newQim = QImage( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB32 );
            break;
        }
        break;
    case 3:
        qDebug() << "case 3:";
        imageLabel->setPixmap(QPixmap::fromImage(newQim));
        break;
    default:
        qDebug() << "default:";
        cap.release();
#ifdef Q_OS_WIN
        cap  = cv::VideoCapture(CV_CAP_ANY);
#endif
#ifdef Q_OS_ANDROID
        cap  = cv::VideoCapture(CV_CAP_ANDROID);
#endif

        if(cap.isOpened()){
           // while( true ){
                cap >> frame;
                if( !frame.empty() ){
                    mat = frame;
//                    newQim = QImage( frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888 );
//                    imageLabel->setPixmap(QPixmap::fromImage(newQim));
                }else{
                    qDebug() << "no frame there...";
                }
//                QEventLoop loop;
//                QTimer::singleShot(10, &loop, SLOT(quit()));
//                loop.exec();
                //qDebug() << "has waited 1 sec..." << i1++;
            //}

        }

        cap.release();
        i0 = 1;
        break;
    }
    i0++;
}




//bool MainWindow::loadFile(const QString &fileName)
//{
//    QImage image(fileName);
//    if (image.isNull()) {
//        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
//                                 tr("Cannot load %1.").arg(QDir::toNativeSeparators(fileName)));
//        setWindowFilePath(QString());
//        imageLabel->setPixmap(QPixmap());
//        imageLabel->adjustSize();
//        return false;
//    }
//    imageLabel->setPixmap(QPixmap::fromImage(image));
//    scaleFactor = 1.0;

//    printAct->setEnabled(true);
//    fitToWindowAct->setEnabled(true);
//    updateActions();

//    if (!fitToWindowAct->isChecked())
//        imageLabel->adjustSize();

//    setWindowFilePath(fileName);
//    return true;
//}


//void MainWindow::normalSize()
//{
//    imageLabel->adjustSize();
//    scaleFactor = 1.0;
//}

//void MainWindow::fitToWindow()
//{
//    bool fitToWindow = fitToWindowAct->isChecked();
//    scrollArea->setWidgetResizable(fitToWindow);
//    if (!fitToWindow) {
//        normalSize();
//    }
//    updateActions();
//}



//void MainWindow::updateActions()
//{
//    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
//    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
//    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
//}

//void MainWindow::scaleImage(double factor)
//{
//    Q_ASSERT(imageLabel->pixmap());
//    scaleFactor *= factor;
//    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

//    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
//    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

//    zoomInAct->setEnabled(scaleFactor < 3.0);
//    zoomOutAct->setEnabled(scaleFactor > 0.333);
//}
