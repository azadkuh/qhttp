################################################################################
#   QBuildSystem
#
#   Copyright(c) 2021 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   Redistribution and use in source and binary forms are allowed under the
#   terms of BSD License 2.0.
################################################################################
CONFIG_TYPE="Test"
include (./configs.pri)

!defined(TEST_NAME, var): TEST_NAME=tst_$$ProjectName

TEMPLATE = app
TARGET=$$TEST_NAME
QT+=testlib
CONFIG      += console
osx:CONFIG  -= app_bundle

DESTDIR      = $$BaseTestBinFolder

include(./common.pri)
