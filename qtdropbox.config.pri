OTHER_FILES += libqtdropbox.pri

target.path = lib/

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
package.path = qtdropbox

#-------------------------------------------------
# install definitions
#-------------------------------------------------
INSTALLS += target package
