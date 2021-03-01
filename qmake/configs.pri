################################################################################
#   QBuildSystem
#
#   Copyright(c) 2021 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   Redistribution and use in source and binary forms are allowed under the
#   terms of BSD License 2.0.
################################################################################
PRJ_BASE_DIR=$$absolute_path(..)
!defined(VERSION, var){
    VersionFile=$$PRJ_BASE_DIR/version.pri
    !exists($$VersionFile): error("**** libsrc: Unable to find version info file $$VersionFile ****")
    include ($$VersionFile)
}

!defined(ProjectName, var): error(ProjectName not specified)
!defined(VERSION, var): error(ProjectVERSION not specified)
!defined(PREFIX, var): PREFIX=~/local
!defined(DONT_BUILD_DEPS, var): DONT_BUILD_DEPS=0

#+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-
CONFIG(debug, debug|release): DEFINES += TARGOMAN_SHOW_DEBUG=1
CONFIG(release){
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE += -O3
}

DEFINES += PROJ_VERSION=$$VERSION

#+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-
contains(QT_ARCH, x86_64){
    LibFolderPattern     = lib64
} else {
    LibFolderPattern     = lib
}
ModulesFolderPattern    = modules
PluginsFolderPattern    = plugins
LibIncludeFolderPattern = include
BinFolderPattern        = bin
BuildFolderPattern      = build
TestBinFolder           = test
UnitTestBinFolder       = unitTest
ConfigFolderPattern     = conf


BaseLibraryFolder        = $$PRJ_BASE_DIR/out/$$LibFolderPattern
BaseModulesFolder        = $$PRJ_BASE_DIR/out/$$ModulesFolderPattern
BasePluginsFolder        = $$PRJ_BASE_DIR/out/$$PluginsFolderPattern
BaseLibraryIncludeFolder = $$PRJ_BASE_DIR/out/$$LibIncludeFolderPattern
BaseBinFolder            = $$PRJ_BASE_DIR/out/$$BinFolderPattern
BaseTestBinFolder        = $$PRJ_BASE_DIR/out/$$TestBinFolder
BaseUnitTestBinFolder    = $$PRJ_BASE_DIR/out/$$UnitTestBinFolder
BaseBuildFolder          = $$PRJ_BASE_DIR/out/$$BuildFolderPattern/$$ProjectName
BaseConfigFolder         = $$PRJ_BASE_DIR/out/$$ConfigFolderPattern

#+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-
INCLUDEPATH += $$PRJ_BASE_DIR \
               $$PRJ_BASE_DIR/src \
               $$PRJ_BASE_DIR/libsrc \
               $$BaseLibraryIncludeFolder \
               $$PREFIX/include \
               $(HOME)/local/include \
               $$DependencyIncludePaths/

#+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-
DependencyLibPaths      +=   $$BaseLibraryFolder \
                             $$PRJ_BASE_DIR/out/lib64 \
                             $$PRJ_BASE_DIR/out/lib \
                             $$PREFIX/lib64 \
                             $$PREFIX/lib \
                             $(HOME)/local/lib \
                             $(HOME)/local/lib64 \

#+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-
win32: DEFINES += _WINDOWS
FullDependencySearchPaths = $$DependencyLibPaths
unix:
  FullDependencySearchPaths+=  /usr/lib \
                               /usr/lib64 \
                               /usr/local/lib \
                               /usr/local/lib64 \
                               /lib/x86_64-linux-gnu


QMAKE_LIBDIR += $$FullDependencySearchPaths

#+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-
defineTest(addSubdirs) {
    for(subdirs, 1) {
        entries = $$files($$subdirs)
        for(entry, entries) {
            name = $$replace(entry, [/\\\\], _)
            SUBDIRS += $$name
            eval ($${name}.subdir = $$entry)
            for(dep, 2):eval ($${name}.depends += $$replace(dep, [/\\\\], _))
            export ($${name}.subdir)
            export ($${name}.depends)
        }
    }
    export (SUBDIRS)
}

#+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-
message("*******************   $$ProjectName BASE CONFIG  ************************ ")
message("* Building $$ProjectName Ver. $$VERSION")
message("* Base Project Path :  $$absolute_path(..)")
message("* Base build target : $$PRJ_BASE_DIR/out")
message("* Install Path      : $$PREFIX/")
message("* Definitions       : $$DEFINES")
message("* DONT_BUILD_DEPS   : $$DONT_BUILD_DEPS")
message("* DISABLED_DEPS     : $$DISABLED_DPES")
message("******************************************************************** ")

!defined(CONFIG_TYPE, var) {
    unix: system($$PRJ_BASE_DIR/qmake/buildDeps.sh $$PRJ_BASE_DIR $$PRJ_BASE_DIR/out/.depsBuilt $$DONT_BUILD_DEPS $$DISABLED_DPES)
    win32: error(submodule auto-compile has not yet been implemented for windows)
}
