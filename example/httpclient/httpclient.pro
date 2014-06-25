QT          += core network
QT          -= gui
CONFIG      += console
CONFIG      += c++11
osx:CONFIG  -= app_bundle

TARGET       = httpclient
TEMPLATE     = app
PRJDIR       = ../..
include($$PRJDIR/commondir.pri)

INCLUDEPATH += ..


SOURCES     += main.cpp \
    httpclient.cpp \
    ../include/gason.cpp

HEADERS     += \
    httpclient.hpp \
    ../include/gason.hpp \
    ../include/jsonbuilder.hpp

LIBS        += $$PRJDIR/xbin/libqhttpserver.a
