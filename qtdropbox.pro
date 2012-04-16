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
    src/qdropboxaccount.cpp \
    src/qdropboxfile.cpp

HEADERS += \
    src/qtdropbox_global.h \
    src/qdropbox.h \
    src/qdropboxjson.h \
    src/qdropboxaccount.h \
    src/qdropboxfile.h \
    src/qtdropbox.h

TARGET = QtDropbox

OTHER_FILES += \
    libqtdropbox.pri
