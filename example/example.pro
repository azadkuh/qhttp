TEMPLATE = subdirs

include(../commondir.pri)

SUBDIRS += helloworld
SUBDIRS += postcollector
SUBDIRS += basic-server
SUBDIRS += ssl-simple

equals(ENABLE_QHTTP_CLIENT, "1") {
    SUBDIRS += keep-alive
}

