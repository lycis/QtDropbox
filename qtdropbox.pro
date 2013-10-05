#-------------------------------------------------
# General definitions and dependencies
#-------------------------------------------------

QT       += network xml

QT       -= gui

TEMPLATE = lib

DEFINES += QTDROPBOX_LIBRARY
#          QTDROPBOX_DEBUG

SOURCES += \
    src/qdropbox.cpp \
    src/qdropboxjson.cpp \
    src/qdropboxaccount.cpp \
    src/qdropboxfile.cpp \
    src/qdropboxfileinfo.cpp

HEADERS += \
    src/qtdropbox_global.h \
    src/qdropbox.h \
    src/qdropboxjson.h \
    src/qdropboxaccount.h \
    src/qdropboxfile.h \
    src/qtdropbox.h \
    src/qdropboxfileinfo.h

TARGET = QtDropbox

include(qtdropbox.config.pri)
