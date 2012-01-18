#-------------------------------------------------
#
# Project created by QtCreator 2012-01-16T19:14:33
#
#-------------------------------------------------

QT       += network xml debug

QT       -= gui

TEMPLATE = lib

DEFINES += QTDROPBOX_LIBRARY \
           QTDROPBOX_DEBUG

SOURCES += \
    src/qdropbox.cpp \
    src/qdropboxjson.cpp \
    src/qdropboxaccount.cpp

HEADERS += \
    src/qtdropbox.h\
    src/qtdropbox_global.h \
    src/qdropbox.h \
    src/qdropboxjson.h \
    src/qdropboxaccount.h

TARGET = QtDropbox

OTHER_FILES += \
    libqtdropbox.pri
