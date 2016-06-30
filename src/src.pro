QT       += core network
QT       -= gui

TARGET    = qhttp
TEMPLATE  = lib

PRJDIR    = ..
include($$PRJDIR/commondir.pri)

!contains(DEFINES, QHTTP_DYNAMIC_LIB) {
    CONFIG += staticlib
}

win32-msvc* {
    # inside library, enables msvc dllexport
    DEFINES *= QHTTP_EXPORT
}

# Joyent http_parser
SOURCES  += $$PRJDIR/3rdparty/http-parser/http_parser.c
HEADERS  += $$PRJDIR/3rdparty/http-parser/http_parser.h

SOURCES  += \
    qhttpabstracts.cpp \
    qhttpserverconnection.cpp \
    qhttpserverrequest.cpp \
    qhttpserverresponse.cpp \
    qhttpserver.cpp

HEADERS  += \
    ../include/qhttp/qhttpfwd.hpp \
    ../include/qhttp/qhttpheaders.hpp \
    ../include/qhttp/qhttpabstracts.hpp \
    ../include/qhttp/qhttpserverconnection.hpp \
    ../include/qhttp/qhttpserverrequest.hpp \
    ../include/qhttp/qhttpserverresponse.hpp \
    ../include/qhttp/qhttpserver.hpp

contains(DEFINES, QHTTP_HAS_CLIENT) {
    SOURCES += \
        qhttpclientrequest.cpp \
        qhttpclientresponse.cpp \
        qhttpclient.cpp

    HEADERS += \
        ../include/qhttp/qhttpclient.hpp \
        ../include/qhttp/qhttpclientresponse.hpp \
        ../include/qhttp/qhttpclientrequest.hpp
}
