QT          += core network
QT          -= gui
CONFIG      += console
CONFIG      += c++11
osx:CONFIG  -= app_bundle

DEFINES     += USETHREADS=1

TARGET       = benchmark
TEMPLATE     = app
PRJDIR       = ../..
include($$PRJDIR/commondir.pri)

INCLUDEPATH += ..


SOURCES     += main.cpp \
    ../include/gason.cpp \
    server.cpp \
    clients.cpp

HEADERS     += \
    ../include/gason.hpp \
    ../include/jsonbuilder.hpp \
    server.hpp \
    clients.hpp

LIBS        += $$PRJDIR/xbin/libqhttp.a
