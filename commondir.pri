# specifying common dirs

unix {
    TEMPDIR         = $$PRJDIR/tmp/unix/$$TARGET
    macx:TEMPDIR    = $$PRJDIR/tmp/osx/$$TARGET
    QMAKE_CFLAGS    += -std=gnu99
    QMAKE_CXXFLAGS  += -std=c++11
}

!unix:error("your platform is not supported!")


DESTDIR         = $$PRJDIR/xbin
MOC_DIR         = $$TEMPDIR
OBJECTS_DIR     = $$TEMPDIR
RCC_DIR         = $$TEMPDIR
UI_DIR          = $$TEMPDIR/Ui
DEPENDPATH      += $$PRJDIR/xbin

INCLUDEPATH     +=  . $$PRJDIR/src $$PRJDIR/3rdparty

