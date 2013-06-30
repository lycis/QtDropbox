macx {
  CONFIG += lib_bundle

  CONFIG(release, debug|release) {
    QMAKE_CFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_OBJECTIVE_CFLAGS_RELEASE =  $$QMAKE_OBJECTIVE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
  }

  QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@executable_path/../Frameworks/
  QMAKE_INFO_PLIST = QtDropbox-Info.plist
  OTHER_FILES += QtDropbox-Info.plist

  HEADER_FILES = src/qtdropbox_global.h \
                 src/qdropbox.h \
                 src/qtdropbox.h \
                 src/qdropboxjson.h \
                 src/qdropboxaccount.h \
                 src/qdropboxfile.h \
                 src/qdropboxfileinfo.h

  QMAKE_FRAMEWORK_VERSION = A
  QTDROPBOX_FRAMEWORK_HEADERS.version = Versions
  QTDROPBOX_FRAMEWORK_HEADERS.path  = Headers
  QTDROPBOX_FRAMEWORK_HEADERS.files = $${HEADER_FILES}

  QMAKE_BUNDLE_DATA += QTDROPBOX_FRAMEWORK_HEADERS
}
else {

  OTHER_FILES += \
      libqtdropbox.pri

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
}