#-------------------------------------------------
#
# Project created by QtCreator 2013-01-29T00:05:24
#
#-------------------------------------------------

QT       += network testlib xml

QT       -= gui

TARGET = qtdropboxtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += qdropboxtest.cpp \
    main.cpp \
    qdropboxjsontest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    qdropboxtest.hpp \
    qdropboxjsontest.hpp

LIBS += -L../lib/
INCLUDEPATH += ../qtdropbox/

include(../libqtdropbox.pri)

target.path = ../lib/
INSTALLS += target
