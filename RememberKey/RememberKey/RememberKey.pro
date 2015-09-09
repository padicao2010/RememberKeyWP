#-------------------------------------------------
#
# Project created by QtCreator 2015-08-29T20:12:50
#
#-------------------------------------------------

QT       += core gui sql network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RememberKey
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    editdialog.cpp \
    keydatabase.cpp \
    keyinfo.cpp \
    ../QtAesLib/qtaes.cpp \
    ../QtOneDriveLib/qtonedrive.cpp \
    onedrivedialog.cpp \
    createdialog.cpp \
    passworddialog.cpp

HEADERS  += mainwindow.h \
    editdialog.h \
    keydatabase.h \
    keyinfo.h \
    ../QtAesLib/qtaes.h \
    ../QtOneDriveLib/qtonedrive.h \
    onedrivedialog.h \
    createdialog.h \
    passworddialog.h

FORMS    += mainwindow.ui \
    editdialog.ui \
    onedrivedialog.ui \
    createdialog.ui \
    passworddialog.ui

unix {
    target.path = /usr/lib
    INSTALLS += target
#   CONFIG +=  link_pkgconfig
#   PKGCONFIG += openssl
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib -L/usr/lib -lcrypto
}

win32 {
    LIBS += -LC:/OpenSSL-Win32/lib/MinGW/ -leay32
    INCLUDEPATH += C:/OpenSSL-Win32/include
}

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

