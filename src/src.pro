QT       += core network
QT       -= gui

TARGET    = qhttp
TEMPLATE  = lib

CONFIG += shared_and_static build_all c++11 c++14

equals(ENABLE_QHTTP_CLIENT, "1") {
    DEFINES *= QHTTP_HAS_CLIENT
}

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

win32-msvc* {
    # inside library, enables msvc dllexport
    DEFINES *= QHTTP_EXPORT
}

INCLUDEPATH += $$PWD/../include $$PWD/../3rdparty

# Joyent http_parser
SOURCES  += $$PWD/../3rdparty/http-parser/http_parser.c
HEADERS  += $$PWD/../3rdparty/http-parser/http_parser.h

SOURCES  += \
    qhttpabstracts.cpp \
    qhttpserverconnection.cpp \
    qhttpserverrequest.cpp \
    qhttpserverresponse.cpp \
    qhttpserver.cpp \
    qhttpsslsocket.cpp

PUBLIC_HEADERS  = \
    ../include/qhttp/qhttpfwd.hpp \
    ../include/qhttp/qhttpheaders.hpp \
    ../include/qhttp/qhttpabstracts.hpp \
    ../include/qhttp/qhttpserverconnection.hpp \
    ../include/qhttp/qhttpserverrequest.hpp \
    ../include/qhttp/qhttpserverresponse.hpp \
    ../include/qhttp/qhttpserver.hpp \
    ../include/qhttp/qhttpsslconfig.hpp \
    ../include/qhttp/qhttpsslsocket.hpp

contains(DEFINES, QHTTP_HAS_CLIENT) {
    SOURCES += \
        qhttpclientrequest.cpp \
        qhttpclientresponse.cpp \
        qhttpclient.cpp

    PUBLIC_HEADERS += \
        ../include/qhttp/qhttpclient.hpp \
        ../include/qhttp/qhttpclientresponse.hpp \
        ../include/qhttp/qhttpclientrequest.hpp
}

HEADERS += $$PUBLIC_HEADERS

unix:!mac {
    CONFIG += create_pc create_prl no_install_prl

    headers.files = $$PUBLIC_HEADERS
    headers.path = $$PREFIX/include/qhttp/
    target.path = $$PREFIX/lib/
    INSTALLS += target headers

    QMAKE_PKGCONFIG_NAME = QHttp
    QMAKE_PKGCONFIG_DESCRIPTION = QHttp is a lightweight, asynchronous and fast HTTP library in c++14 / Qt5
    QMAKE_PKGCONFIG_PREFIX = $$PREFIX
    QMAKE_PKGCONFIG_LIBDIR = $$target.path
    QMAKE_PKGCONFIG_INCDIR = $$headers.path
    QMAKE_PKGCONFIG_VERSION = 2.0.0
    QMAKE_PKGCONFIG_DESTDIR = pkgconfig
}
