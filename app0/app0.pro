QT += core gui widgets

HEADERS       = mainWindow.h \
    mylabel.h
SOURCES       = mainWindow.cpp \
                main.cpp \
    mylabel.cpp


CONFIG += mobility

TEMPLATE = app

TARGET = app0

RESOURCES += \
    res.qrc

win32{
    INCLUDEPATH += "C:/dev/opencv/build_qt/install/include"
    LIBS += -L"C:/dev/opencv/build_qt/bin"\
        libopencv_core2410d \
        libopencv_highgui2410d \
        libopencv_imgproc2410d \
        libopencv_features2d2410d \
        libopencv_calib3d2410d \
}
android {
    INCLUDEPATH += "C:/dev/opencv/build_android/sdk/native/jni/include"

    LIBS += \
        -L"c:/dev/opencv/build_android/sdk/native/3rdparty/libs/armeabi-v7a"\
        -L"c:/dev/opencv/build_android/sdk/native/libs/armeabi-v7a"\
        -llibtiff\
        -llibjpeg\
        -llibjasper\
        -llibpng\
        -lIlmImf\
        -ltbb\
        -lopencv_core\
        -lopencv_androidcamera\
        -lopencv_flann\
        -lopencv_imgproc\
        -lopencv_highgui\
        -lopencv_features2d\
        -lopencv_calib3d\
        -lopencv_ml\
        -lopencv_objdetect\
        -lopencv_video\
        -lopencv_contrib\
        -lopencv_photo\
        -lopencv_java\
        -lopencv_legacy\
        -lopencv_ocl\
        -lopencv_stitching\
        -lopencv_superres\
        -lopencv_ts\
        -lopencv_videostab


    DISTFILES += \
        android/gradle/wrapper/gradle-wrapper.jar \
        android/AndroidManifest.xml \
        android/res/values/libs.xml \
        android/build.gradle \
        android/gradle/wrapper/gradle-wrapper.properties \
        android/gradlew \
        android/gradlew.bat

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}
