#-------------------------------------------------
#
# Project created by QtCreator 2014-09-29T13:04:10
#
#-------------------------------------------------

QT       += network core

TARGET = QtOneDriveLib
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++11

DEFINES += QTONEDRIVELIB_LIBRARY

SOURCES += qtonedrive.cpp

HEADERS += qtonedrive.h\
        qtonedrivelib_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
}

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

