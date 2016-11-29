# specifying common dirs

# comment following line to build the lib as static library
DEFINES *= QHTTP_DYNAMIC_LIB
# comment following line to trim client classes from build
DEFINES *= QHTTP_HAS_CLIENT
# Qt5.5.1 on OSX needs both c++11 and c++14!! the c++14 is not enough
CONFIG  += c++11 c++14

unix {
    TEMPDIR      = $$PRJDIR/tmp/unix/$$TARGET
    macx:TEMPDIR = $$PRJDIR/tmp/osx/$$TARGET
    # add application working directory to RPATH
    # helps finding the *.so files in app's working dir
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
}

win32 {
    TEMPDIR  = $$PRJDIR/tmp/win32/$$TARGET
    DEFINES += _WINDOWS WIN32_LEAN_AND_MEAN NOMINMAX
}


DESTDIR      = $$PRJDIR/xbin
MOC_DIR      = $$TEMPDIR
OBJECTS_DIR  = $$TEMPDIR
RCC_DIR      = $$TEMPDIR
UI_DIR       = $$TEMPDIR/Ui
LIBS        += -L$$PRJDIR/xbin

INCLUDEPATH +=  . $$PRJDIR/include $$PRJDIR/3rdparty

equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 4) {
  CONFIG -= c++11 c++14
  QMAKE_CXXFLAGS += -std=c++14
}
