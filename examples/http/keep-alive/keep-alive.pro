QT          += core network
QT          -= gui
osx:CONFIG  -= app_bundle

TARGET       = keep-alive
TEMPLATE     = app

PRJDIR       = ../..
include($$PRJDIR/commondir.pri)

HEADERS   +=

SOURCES   += main.cpp

LIBS      += -lqhttp
