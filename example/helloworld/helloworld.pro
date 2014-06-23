QT          += core network
QT          -= gui
osx:CONFIG  -= app_bundle

TARGET       = helloworld
TEMPLATE     = app
CONFIG      += c++11

PRJDIR       = ../..
include($$PRJDIR/commondir.pri)

HEADERS   +=

SOURCES   += main.cpp


LIBS      += $$PRJDIR/xbin/libqhttpserver.a
