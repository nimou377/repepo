#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include <QMainWindow>
//#include <QPixmap>
//#include <QImage>

#include "mylabel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void mySlot(int x,int y);

private:
    MyLabel *imageLabel;
    QImage qim,newQim;
    cv::Mat mat,grayMat;
    QVector<QRgb>  sColorTable;

    cv::VideoCapture cap;
    cv::Mat frame;

    bool b0,b1,b2,b3;
    int i0,i1,i2,i3;
};


#endif // MAINWINDOW_H
