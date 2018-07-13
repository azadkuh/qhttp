QT          += core network
QT          -= gui
CONFIG      += console
osx:CONFIG  -= app_bundle

TARGET       = helloworld
TEMPLATE     = app

PRJDIR       = ../..
include($$PRJDIR/commondir.pri)

HEADERS   +=

SOURCES   += main.cpp

LIBS      += -lqhttp
