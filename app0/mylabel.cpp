#include "mylabel.h"

#include <QLabel>
#include <QMouseEvent>

#include <QDebug>

MyLabel::MyLabel(const QString& text,QWidget* parent)
:QLabel(text,parent){}
MyLabel::MyLabel(QWidget* parent)
:QLabel(parent){}

void MyLabel::mouseReleaseEvent(QMouseEvent * evt){
    emit clicked(evt->x(),evt->y());
}
