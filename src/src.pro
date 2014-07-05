QT       += core network
#QT       += core-private network-private
QT       -= gui

TARGET    = qhttpserver
TEMPLATE  = lib
CONFIG   += staticlib
CONFIG   += c++11

PRJDIR    = ..
include($$PRJDIR/commondir.pri)

DEFINES += QHTTP_MEMORY_LOG=2 QHTTP_MESSAGES_LOG=0


SOURCES += \
    qhttpabstracts.cpp \
    qhttpconnection.cpp \
    qhttprequest.cpp \
    qhttpresponse.cpp \
    qhttpserver.cpp

HEADERS += \
    qhttpserverapi.hpp \
    qhttpserverfwd.hpp \
    qhttpabstracts.hpp \
    qhttpconnection.hpp \
    qhttprequest.hpp \
    qhttpresponse.hpp \
    qhttpserver.hpp \
    private/qhttpbase.hpp \
    private/qhttpconnection_private.hpp \
    private/qhttprequest_private.hpp \
    private/qhttpresponse_private.hpp \
    private/qhttpserver_private.hpp

SOURCES += $$PRJDIR/3rdparty/http-parser/http_parser.c
HEADERS += $$PRJDIR/3rdparty/http-parser/http_parser.h

