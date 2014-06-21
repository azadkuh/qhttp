QT       += core network
QT       -= gui

TARGET    = qhttpserver
TEMPLATE  = lib
CONFIG   += staticlib
CONFIG   += c++11

PRJDIR    = ..
include($$PRJDIR/commondir.pri)



SOURCES += \
    qhttpconnection.cpp \
    qhttprequest.cpp \
    qhttpresponse.cpp \
    qhttpserver.cpp

HEADERS += \
    qhttpconnection.hpp \
    qhttprequest.hpp \
    qhttpresponse.hpp \
    qhttpserver.hpp \
    qhttpserverapi.hpp \
    qhttpserverfwd.hpp

SOURCES += $$PRJDIR/3rdparty/http-parser/http_parser.c
HEADERS += $$PRJDIR/3rdparty/http-parser/http_parser.h

