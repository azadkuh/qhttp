QT       += core network
QT       -= gui

TARGET    = qhttpserver
TEMPLATE  = lib
CONFIG   += staticlib
CONFIG   += c++11

PRJDIR    = ..
include($$PRJDIR/commondir.pri)

DEFINES  += QHTTP_MEMORY_LOG=0 QHTTP_MESSAGES_LOG=0


SOURCES  += \
    qhttpabstracts.cpp \
    qhttpserverconnection.cpp \
    qhttpserverrequest.cpp \
    qhttpserverresponse.cpp \
    qhttpserver.cpp \
    qhttpclientrequest.cpp \
    qhttpclientresponse.cpp \
    qhttpclient.cpp

HEADERS  += \
    qhttpfwd.hpp \
    qhttpabstracts.hpp \
    qhttpserverconnection.hpp \
    qhttpserverrequest.hpp \
    qhttpserverresponse.hpp \
    qhttpserver.hpp \
    qhttpclient.hpp \
    qhttpclientresponse.hpp \
    qhttpclientrequest.hpp \
    private/qhttpbase.hpp \
    private/qhttpserverconnection_private.hpp \
    private/qhttpserverrequest_private.hpp \
    private/qhttpserverresponse_private.hpp \
    private/qhttpserver_private.hpp \
    private/qhttpclientrequest_private.hpp \
    private/qhttpclientresponse_private.hpp \
    private/qhttpclient_private.hpp

SOURCES  += $$PRJDIR/3rdparty/http-parser/http_parser.c
HEADERS  += $$PRJDIR/3rdparty/http-parser/http_parser.h

