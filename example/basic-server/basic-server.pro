QT          += core network
QT          -= gui
osx:CONFIG  -= app_bundle

TARGET       = basic-server
TEMPLATE     = app
CONFIG      += c++11

PRJDIR       = ../..
include($$PRJDIR/commondir.pri)

HEADERS   +=

SOURCES   += \
    main.cpp


#LIBS      += $$PRJDIR/xbin/libqhttp.a
LIBS      += -L$$PRJDIR/xbin -lqhttp
