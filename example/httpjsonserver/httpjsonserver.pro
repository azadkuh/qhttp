QT          += core network
QT          -= gui
CONFIG      += console
CONFIG      += c++11
osx:CONFIG  -= app_bundle

TARGET       = httpjsonserver
TEMPLATE     = app
PRJDIR       = ../..
include($$PRJDIR/commondir.pri)

INCLUDEPATH += ..


SOURCES     += main.cpp \
    httpserver.cpp \
    ../include/gason.cpp

HEADERS     += \
    httpserver.hpp \
    ../include/gason.hpp \
    ../include/jsonbuilder.hpp

LIBS        += $$PRJDIR/xbin/libqhttpserver.a
