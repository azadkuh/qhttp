QT          += core network
QT          -= gui
CONFIG      += console
osx:CONFIG  -= app_bundle

TARGET       = ssl-simple
TEMPLATE     = app

HEADERS   +=

SOURCES   += main.cpp
RESOURCES += resources.qrc

include($$PWD/../example.pri)
