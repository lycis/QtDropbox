QT       += network xml
QT       -= gui

CONFIG += static

TEMPLATE = lib

DEFINES += QTDROPBOX_LIBRARY

SOURCES += \
    qdropbox.cpp \
    qdropboxjson.cpp \
    qdropboxaccount.cpp \
    qdropboxfile.cpp \
    qdropboxfileinfo.cpp \
    qdropboxdeltaresponse.cpp

HEADERS += \
    qtdropbox_global.h \
    qdropbox.h \
    qdropboxjson.h \
    qdropboxaccount.h \
    qdropboxfile.h \
    qtdropbox.h \
    qdropboxfileinfo.h \
    qdropboxdeltaresponse.h

TARGET = qtdropbox

documentation.commands = doxygen doc/doxygen.conf
QMAKE_EXTRA_TARGETS += documentation

package.files = libqtdropbox.pri \
                src/*.h
package.path = qtdropbox

INSTALLS += target package
