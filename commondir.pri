# specifying common dirs

DEFINES *= QHTTP_HAS_CLIENT
# Qt5.5.1 on OSX needs both c++11 and c++14!! the c++14 is not enough
CONFIG  += c++11 c++14

unix {
    TEMPDIR         = $$PRJDIR/tmp/unix/$$TARGET
    macx:TEMPDIR    = $$PRJDIR/tmp/osx/$$TARGET
}

win32 {
    warning("Windows port of this library has not been tested nor profiled.")
    TEMPDIR         = $$PRJDIR/tmp/win32/$$TARGET
    DEFINES        += _WINDOWS WIN32_LEAN_AND_MEAN NOMINMAX
}


DESTDIR      = $$PRJDIR/xbin
MOC_DIR      = $$TEMPDIR
OBJECTS_DIR  = $$TEMPDIR
RCC_DIR      = $$TEMPDIR
UI_DIR       = $$TEMPDIR/Ui
LIBS        += -L$$PRJDIR/xbin

INCLUDEPATH +=  . $$PRJDIR/src $$PRJDIR/3rdparty

