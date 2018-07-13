load(qt_build_config)

INCLUDEPATH += $$PWD $$PWD/src $$PWD/3rdparty
DEPENDPATH += $$PWD

QT += core network
QT -= gui

TARGET    = QtHttp
MODULE_PRI = ../../modules/qt_http.pri
MODULE = http

load(qt_module)

DEFINES       *= QHTTP_MEMORY_LOG=0
win32:DEFINES *= QHTTP_LIBRARY

# Joyent http_parser
SOURCES  += $$PWD/3rdparty/http-parser/http_parser.c
HEADERS  += $$PWD/3rdparty/http-parser/http_parser.h

SOURCES  += \
    qhttpabstracts.cpp \
    qhttpserverconnection.cpp \
    qhttpserverrequest.cpp \
    qhttpserverresponse.cpp \
    qhttpserver.cpp

HEADERS  += \
    qhttpfwd.h \
    qhttpabstracts.h \
    qhttpserverconnection.h \
    qhttpserverrequest.h \
    qhttpserverresponse.h \
    qhttpserver.h
