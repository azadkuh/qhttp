################################################################################
#   QBuildSystem
#
#   Copyright(c) 2021 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   Redistribution and use in source and binary forms are allowed under the
#   terms of BSD License 2.0.
################################################################################
MOC_DIR      = $$BaseBuildFolder/$$TARGET/moc
OBJECTS_DIR  = $$BaseBuildFolder/$$TARGET/obj
RCC_DIR      = $$BaseBuildFolder/$$TARGET/rcc
UI_DIR       = $$BaseBuildFolder/$$TARGET/ui

INCLUDEPATH  += $$PRJ_BASE_DIR/libsrc
INCLUDEPATH  += $$PRJ_BASE_DIR/libsrc/lib$$ProjectName/Private/
INCLUDEPATH  += $$PRJ_BASE_DIR/libsrc/lib$$ProjectName/
QMAKE_LIBDIR += $$BaseLibraryFolder

equals(CONFIG_TYPE, Test): LIBS += -l$$ProjectName
equals(CONFIG_TYPE, UnitTest): LIBS += -l$$ProjectName

message("*******  $$ProjectName $$VERSION $$CONFIG_TYPE *************** ")
message("* Base build path   : $$BaseBuildFolder/$$TARGET")
message("* Target Path       : $$DESTDIR")
message("* Install Path      : $$PREFIX")
message("* Library build type: $$LIB_TYPE ")
message("* DEFINES           : $$DEFINES")
message("******************************************************************** ")

