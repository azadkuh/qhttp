QT       += core network
QT       -= gui

TARGET    = qhttpserver
TEMPLATE  = lib
CONFIG   += staticlib
CONFIG   += c++11

PRJDIR    = ..
include($$PRJDIR/commondir.pri)

DEFINES += QHTTPSERVER_MEMORY_LOG=0 QHTTPSERVER_MESSAGES_LOG=0


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
    qhttpserverfwd.hpp \
    private/qhttpresponse_private.hpp \
    private/qhttprequest_private.hpp \
    private/qhttpconnection_private.hpp \
    private/qhttpbase.hpp \
    private/qhttpserver_private.hpp

SOURCES += $$PRJDIR/3rdparty/http-parser/http_parser.c
HEADERS += $$PRJDIR/3rdparty/http-parser/http_parser.h

