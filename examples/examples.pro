################################################################################
#   QBuildSystem
#
#   Copyright(c) 2021 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   Redistribution and use in source and binary forms are allowed under the
#   terms of BSD License 2.0.
################################################################################
include(../version.pri)
TEMPLATE = subdirs

SUBDIRS += postcollector
SUBDIRS += basic-server
SUBDIRS += simpleRESTServer

contains(DEFINES, QHTTP_HAS_CLIENT) {
    SUBDIRS += keep-alive
    SUBDIRS += helloworld
}

