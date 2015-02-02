#ifndef CPP_TOOLS
#define CPP_TOOLS


#include <QObject>
#include <QCamera>

#include <iostream>

class Cpp_tools : public QObject{
   Q_OBJECT
public:
    explicit Cpp_tools (QObject* parent = 0) : QObject(parent) {}


    Q_INVOKABLE void f0(){
//        QObject * obj = rootview->rootObject()->findChild<QObject *>("camera");
//        QVariant mediaObject = obj->property("mediaObject");
//        camera = qvariant_cast<QCamera *>(mediaObject);
    }
    Q_INVOKABLE void f1(){

    }
    Q_INVOKABLE void f2(){

    }
    Q_INVOKABLE void f3(){

    }

    QCamera * camera;
};

#endif // CPP_TOOLS

