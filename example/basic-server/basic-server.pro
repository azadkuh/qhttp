QT          += core network
QT          -= gui
osx:CONFIG  -= app_bundle

TARGET       = basic-server
TEMPLATE     = app
CONFIG      += c++11

PRJDIR       = ../..
include($$PRJDIR/commondir.pri)

HEADERS   += \
    httpserver.hpp


SOURCES   += \
    main.cpp \
    httpserver.cpp


LIBS      += $$PRJDIR/xbin/libqhttpserver.a
