QT       += core network
QT       -= gui

TARGET    = qhttp
TEMPLATE  = lib

PRJDIR    = ..
include($$PRJDIR/commondir.pri)

DEFINES       *= QHTTP_MEMORY_LOG=0
win32:DEFINES *= QHTTP_EXPORT

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
    qhttpfwd.hpp \
    qhttpabstracts.hpp \
    qhttpserverconnection.hpp \
    qhttpserverrequest.hpp \
    qhttpserverresponse.hpp \
    qhttpserver.hpp

contains(DEFINES, QHTTP_HAS_CLIENT) {
    SOURCES += \
        qhttpclientrequest.cpp \
        qhttpclientresponse.cpp \
        qhttpclient.cpp

    HEADERS += \
        qhttpclient.hpp \
        qhttpclientresponse.hpp \
        qhttpclientrequest.hpp
}

!contains(CONFIG, no_install) {
    INSTALL_PREFIX = $$[QT_INSTALL_HEADERS]/qhttp
    INSTALL_HEADERS = $$HEADERS
    include(qmake/headerinstall.pri)

    target = $$TARGET
    target.path = $$[QT_INSTALL_LIBS]

    INSTALLS += target
}
