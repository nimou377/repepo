
#include <QtWidgets>

#include "mainWindow.h"


MainWindow::MainWindow()
{
    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    setCentralWidget(imageLabel);

    //resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
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
