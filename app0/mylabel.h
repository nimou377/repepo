#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>

class MyLabel : public QLabel{
    Q_OBJECT
public:
    MyLabel(const QString& text,QWidget* parent=0);
    MyLabel(QWidget* parent=0);
    ~MyLabel(){}
signals:
    void clicked(int x, int y);
protected:
    void mouseReleaseEvent(QMouseEvent * evt);
};

#endif // MYLABEL_H
