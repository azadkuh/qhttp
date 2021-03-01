################################################################################
#   QBuildSystem
#
#   Copyright(c) 2021 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   Redistribution and use in source and binary forms are allowed under the
#   terms of BSD License 2.0.
################################################################################
CONFIG_TYPE="library"
include (./configs.pri)

TEMPLATE = lib
CONFIG += plugin
defined(PluginName): TARGET = $$PluginName
!defined(PluginName): TARGET = $$ProjectName

QMAKE_CXXFLAGS_RELEASE += -fPIC
QMAKE_CXXFLAGS_DEBUG += -fPIC

equals(LIB_TYPE, static) {
    DEFINES += TARGOMAN_BUILD_STATIC
    CONFIG+=staticlib
} else {
    DEFINES += TARGOMAN_BUILD_SHARED
    LIB_TYPE  = shared
}

HEADERS += $$DIST_HEADERS \
           $$PRIVATE_HEADERS \
           $$SUBMODULE_HEADERS \

DESTDIR      = $$BaseFolder

include(./common.pri)



 
