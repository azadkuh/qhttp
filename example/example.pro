TEMPLATE = subdirs

SUBDIRS += helloworld \
    uStaticServer
SUBDIRS += postcollector
SUBDIRS += basic-server

contains(DEFINES, QHTTP_HAS_CLIENT) {
    SUBDIRS += keep-alive
}

