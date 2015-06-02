#-------------------------------------------------
#
# Project created by QtCreator 2013-04-25T12:34:37
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = beeworld2
TEMPLATE = app

#INCLUDEPATH += /usr/include/unicap

#QMAKE_CXXFLAGS += -fopenmp
QMAKE_MAC_SDK = macosx10.9

SOURCES += main.cpp\
        beeworldwindow.cpp \
    beeworld.cpp \
    spinemlnetworkserver.cpp \
    sceneobject.cpp \
    cylinder.cpp \
    plane.cpp \
    sphere.cpp \
    scenetexture.cpp \
    checked.cpp \
    radial.cpp \
    beeworldgiger.cpp

HEADERS  += beeworldwindow.h \
    beeworld.h \
    spinemlnetworkserver.h \
    sceneobject.h \
    cylinder.h \
    plane.h \
    sphere.h \
    scenetexture.h \
    checked.h \
    radial.h \
    beeworldgiger.h \
    gigerdata.h

FORMS    += beeworldwindow.ui


#LIBS += -fopenmp

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml
