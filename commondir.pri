# specifying common dirs

unix {
    TEMPDIR         = $$PRJDIR/tmp/unix/$$TARGET
    macx:TEMPDIR    = $$PRJDIR/tmp/osx/$$TARGET
    QMAKE_CFLAGS    += -std=gnu99
    QMAKE_CXXFLAGS  += -std=c++11
}

win32 {
    warning("Windows port of this library has not been tested nor profiled.")
    TEMPDIR         = $$PRJDIR/tmp/win32/$$TARGET
    CONFIG         += c++11
    DEFINES        += _WINDOWS WIN32_LEAN_AND_MEAN NOMINMAX
}


DESTDIR         = $$PRJDIR/xbin
MOC_DIR         = $$TEMPDIR
OBJECTS_DIR     = $$TEMPDIR
RCC_DIR         = $$TEMPDIR
UI_DIR          = $$TEMPDIR/Ui
LIBS           += -L$$PRJDIR/xbin

INCLUDEPATH     +=  . $$PRJDIR/src $$PRJDIR/3rdparty

