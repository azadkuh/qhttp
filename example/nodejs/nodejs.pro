QT          += core network
QT          -= gui
CONFIG      += console
CONFIG      += c++11
osx:CONFIG  -= app_bundle

TARGET       = nodejs-like
TEMPLATE     = app
PRJDIR       = ../..
include($$PRJDIR/commondir.pri)

INCLUDEPATH += ..


SOURCES     += main.cpp \
    ../include/gason.cpp

HEADERS     += \
    ../include/gason.hpp \
    ../include/jsonbuilder.hpp

#LIBS      += $$PRJDIR/xbin/libqhttp.a
LIBS      += -L$$PRJDIR/xbin -lqhttp
