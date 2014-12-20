QT       += network xml

INCLUDEPATH += $$PWD/src

SOURCES += \
    $$PWD/src/qdropbox.cpp \
    $$PWD/src/qdropboxjson.cpp \
    $$PWD/src/qdropboxaccount.cpp \
    $$PWD/src/qdropboxfile.cpp \
    $$PWD/src/qdropboxfileinfo.cpp \
    $$PWD/src/qdropboxdeltaresponse.cpp

HEADERS += \
    $$PWD/src/qtdropbox_global.h \
    $$PWD/src/qdropbox.h \
    $$PWD/src/qdropboxjson.h \
    $$PWD/src/qdropboxaccount.h \
    $$PWD/src/qdropboxfile.h \
    $$PWD/src/qtdropbox.h \
    $$PWD/src/qdropboxfileinfo.h \
    $$PWD/src/qdropboxdeltaresponse.h

CONFIG += network
