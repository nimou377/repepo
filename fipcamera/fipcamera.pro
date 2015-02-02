#
# Fast image processing demo
# Harald Meyer, <dev@meh.at>, http://www.meh.at
#

VERSION = 1.0.0
QT += network webkit declarative quick qml multimedia widgets opengl

symbian: {
    vendorinfo = "%{\"Harald Meyer-EN\"}" ":\"Harald Meyer\""
    my_deployment.pkg_prerules = vendorinfo
    DEPLOYMENT += my_deployment

    TARGET.UID3 = 0xE6B187F6

    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000

    DEPLOYMENT.display_name += FIP Camera

    TARGET.CAPABILITY += LocalServices \  # camera
         ReadUserData \                   #
         WriteUserData \                  # writing image file
         UserEnvironment                  # camera
}

CONFIG += qt-components
CONFIG += mobility
MOBILITY += multimedia


SOURCES += main.cpp \
    fipmain.cpp \
    customcamera.cpp \
    videosurface.cpp \
    fipthread.cpp \
    blackandwhiteeffect.cpp
HEADERS += \
    fipmain.h \
    customcamera.h \
    videosurface.h \
    fipthread.h \
    blackandwhiteeffect.h
FORMS +=

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/manifest.aegis \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog \
    qml/MainView.qml \
    qml/CameraView.qml

RESOURCES += \
    resources.qrc
