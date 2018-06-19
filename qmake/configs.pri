include(version.pri)
# comment following line to build the lib as static library
DEFINES *= QHTTP_DYNAMIC_LIB
# comment following line to trim client classes from build
DEFINES *= QHTTP_HAS_CLIENT
# Qt5.5.1 on OSX needs both c++11 and c++14!! the c++14 is not enough
CONFIG  += c++11 c++14
# set following definition to 1 in order to log activity
DEFINES       *= QHTTP_MEMORY_LOG=0


win32: DEFINES += _WINDOWS WIN32_LEAN_AND_MEAN NOMINMAX

MOC_DIR      = $$PRJDIR/out/tmp/$$TARGET/moc
OBJECTS_DIR  = $$PRJDIR/out/tmp/$$TARGET/obj
RCC_DIR      = $$PRJDIR/out/tmp/$$TARGET/rcc
UI_DIR       = $$PRJDIR/out/tmp/$$TARGET/ui

LIBS        += -L$$PRJDIR/out/lib

isEqual(TEMPLATE, "lib"){
    DESTDIR      = $$PRJDIR/out/lib
}else{
    DESTDIR      = $$PRJDIR/out/bin
}

INCLUDEPATH +=  . $$PRJDIR/src $$PRJDIR/3rdParty

