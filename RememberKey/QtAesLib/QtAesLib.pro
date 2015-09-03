#-------------------------------------------------
#
# Project created by QtCreator 2015-08-31T11:08:24
#
#-------------------------------------------------

QT       -= gui

TARGET = QtAesLib
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++11

SOURCES += qtaes.cpp

HEADERS += qtaes.h
unix {
    target.path = /usr/lib
    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
}

win32 {
    LIBS += -LC:/OpenSSL-Win32/lib/MinGW/ -leay32
    INCLUDEPATH += C:/OpenSSL-Win32/include
}

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

