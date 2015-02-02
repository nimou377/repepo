INCLUDEPATH += viewfinder

CONFIG += mobility
MOBILITY += multimedia

symbian {
    TARGET.CAPABILITY = UserEnvironment   # To access the camera
}

HEADERS += \
    viewfinder/viewfinderwrapper.h \
    viewfinder/videosurface.h \
    viewfinder/processingthread.h

SOURCES += \
    viewfinder/viewfinderwrapper.cpp \
    viewfinder/videosurface.cpp \
    viewfinder/processingthread.cpp
