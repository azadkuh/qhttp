QT       += core network
QT       -= gui

TARGET    = qhttp
TEMPLATE  = lib

PRJDIR    = ..
include($$PRJDIR/qmake/configs.pri)

win32:DEFINES *= QHTTP_EXPORT

# nodejs http_parser
SOURCES  += $$PRJDIR/3rdParty/http-parser/http_parser.c
SUBMODULE_HEADERS  += $$PRJDIR/3rdParty/http-parser/http_parser.h

SOURCES  += \
    qhttpabstracts.cpp \
    qhttpserverconnection.cpp \
    qhttpserverrequest.cpp \
    qhttpserverresponse.cpp \
    qhttpserver.cpp

DIST_HEADERS  += \
    qhttpfwd.hpp \
    qhttpabstracts.hpp \
    qhttpserverconnection.hpp \
    qhttpserverrequest.hpp \
    qhttpserverresponse.hpp \
    qhttpserver.hpp \
    QHttpServer

contains(DEFINES, QHTTP_HAS_CLIENT) {
    SOURCES += \
        qhttpclientrequest.cpp \
        qhttpclientresponse.cpp \
        qhttpclient.cpp

    DIST_HEADERS += \
        qhttpclient.hpp \
        qhttpclientresponse.hpp \
        qhttpclientrequest.hpp \
        QHttpClient
}

PRIVATE_HEADERS += \
    private/httpparser.hxx \
    private/httpreader.hxx \
    private/httpwriter.hxx \
    private/qhttpabstractsocket.hpp \
    private/qhttpbase.hpp \
    private/qhttpclient_private.hpp \
    private/qhttpclientrequest_private.hpp \
    private/qhttpclientresponse_private.hpp \
    private/qhttpserver_private.hpp \
    private/qhttpserverconnection_private.hpp \
    private/qhttpserverrequest_private.hpp \
    private/qhttpserverresponse_private.hpp

HEADERS += $$DIST_HEADERS \
           $$PRIVATE_HEADERS \
           $$SUBMODULE_HEADERS

QMAKE_CXXFLAGS += -Wno-unknown-pragmas -Wno-padded

include($$PRJDIR/qmake/install.pri)
