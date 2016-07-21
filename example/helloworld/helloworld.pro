QT          += core network
QT          -= gui
CONFIG      += console
osx:CONFIG  -= app_bundle

TARGET       = helloworld
TEMPLATE     = app

HEADERS   +=

SOURCES   += main.cpp

include($$PWD/../example.pri)
