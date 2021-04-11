################################################################################
#   QBuildSystem
#
#   Copyright(c) 2021 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   Redistribution and use in source and binary forms are allowed under the
#   terms of BSD License 2.0.
################################################################################
include($$BASE_PROJECT_PATH/version.pri)
# nodejs http_parser
SOURCES  += ../3rdParty/http-parser/http_parser.c
SUBMODULE_HEADERS  += ../3rdParty/http-parser/http_parser.h
INCLUDEPATH += ../3rdParty/

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

contains(DEFINES, "QHTTP_HAS_CLIENT") {
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

################################################################################
include($$QBUILD_PATH/templates/libConfigs.pri)
