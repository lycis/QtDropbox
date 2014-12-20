#-------------------------------------------------
#
# Project created by QtCreator 2013-01-29T00:05:24
#
#-------------------------------------------------

QT       += network testlib xml gui

TARGET = qtdropboxtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    qtdropboxtest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    qtdropboxtest.hpp \
    keys.hpp \
    keys.hpp

LIBS += -L../../build-qtdropbox-Desktop-Debug
INCLUDEPATH += ../src/

include(../libqtdropbox.pri)

target.path = ../lib/
INSTALLS += target
