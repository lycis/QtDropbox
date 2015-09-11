OTHER_FILES += libqtdropbox.pri

!unix {
    target.path = lib/
}

unix {
    target.path = /usr/lib/
}

#-------------------------------------------------
# Documentation target
#-------------------------------------------------
documentation.commands = doxygen doc/doxygen.conf
QMAKE_EXTRA_TARGETS += documentation

#-------------------------------------------------
# Package target
#-------------------------------------------------
package.files = libqtdropbox.pri \
                src/*.h

!unix {
    package.path = qtdropbox
}

unix {
    package.path = /usr/include/qtdropbox
}

#-------------------------------------------------
# install definitions
#-------------------------------------------------
INSTALLS += target package
