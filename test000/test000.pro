TEMPLATE = app

QT += qml quick widgets

SOURCES += main.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

CONFIG += mobility

INCLUDEPATH += C:\\dev\\opencv\\build_qt\\install\\include
LIBS += -LC:\\dev\\opencv\\build_qt\\bin \
    libopencv_core2410d \
    libopencv_highgui2410d \
    libopencv_imgproc2410d \
    libopencv_features2d2410d \
    libopencv_calib3d2410d \
