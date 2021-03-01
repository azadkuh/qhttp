################################################################################
#   QBuildSystem
#
#   Copyright(c) 2021 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   Redistribution and use in source and binary forms are allowed under the
#   terms of BSD License 2.0.
################################################################################
ProjectName="qhttp"
LIB_PREFIX="QHttp"
VERSION+=4.1.0

win32:DEFINES *= QHTTP_EXPORT

# comment following line to build the lib as static library
DEFINES *= QHTTP_DYNAMIC_LIB
# comment following line to trim client classes from build
DEFINES *= QHTTP_HAS_CLIENT

# set following definition to 1 in order to log activity
DEFINES       *= QHTTP_MEMORY_LOG=0

win32: DEFINES += _WINDOWS WIN32_LEAN_AND_MEAN NOMINMAX
DONT_BUILD_DEPS = 1

#+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-
QT+= core network
QT -= gui
QMAKE_CXXFLAGS += -Wno-unknown-pragmas -Wno-padded

#+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-
# Qt5.5.1 on OSX needs both c++11 and c++14!! the c++14 is not enough
QMAKE_CXXFLAGS += -std=c++11 -std=c++14
CONFIGS += c++11 c++14
