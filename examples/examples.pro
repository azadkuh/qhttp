include(../qmake/configs.pri)
TEMPLATE = subdirs

SUBDIRS += helloworld
SUBDIRS += postcollector
SUBDIRS += basic-server
SUBDIRS += simpleRESTServer

contains(DEFINES, QHTTP_HAS_CLIENT) {
    SUBDIRS += keep-alive
}

