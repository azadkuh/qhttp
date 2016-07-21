QT          += core network
QT          -= gui
CONFIG      += console
osx:CONFIG  -= app_bundle

TARGET       = postcollector
TEMPLATE     = app

PRJDIR       = ../..

HEADERS   +=

SOURCES   += main.cpp

include($$PWD/../example.pri)
