TEMPLATE = lib

QT       += network xml
QT       -= gui

CONFIG += static

DEFINES += QTDROPBOX_LIBRARY

SOURCES += \
    dropbox.cpp \
    dropboxaccount.cpp \
    dropboxdeltaresponse.cpp \
    dropboxfile.cpp \
    dropboxfileinfo.cpp \
    dropboxjson.cpp

HEADERS += \
    dropbox.h \
    dropboxaccount.h \
    dropboxdeltaresponse.h \
    dropboxfile.h \
    dropboxfileinfo.h \
    dropboxjson.h \
    dropbox_global.h

TARGET = qtdropbox

