################################################################################
#   QBuildSystem
#
#   Copyright(c) 2021 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   Redistribution and use in source and binary forms are allowed under the
#   terms of BSD License 2.0.
################################################################################
!contains(CONFIG, no_install) {
    INCLUDE_PREFIX   = $$[QT_INSTALL_HEADERS]/$$LIB_PREFIX
    TARGET_PREFIX    = $$[QT_INSTALL_LIBS]
    EXAMPLES_PREFIX  = $$[QT_INSTALL_EXAMPLES]

    unix:!isEmpty(PREFIX){
            INCLUDE_PREFIX = $$PREFIX/include/$$LIB_PREFIX
            contains(QT_ARCH, x86_64){
                TARGET_PREFIX     = $$PREFIX/lib64
            } else {
                TARGET_PREFIX     = $$PREFIX/lib
            }
    }

    PRJ_BASE_DIR = $${dirname(PWD)}
    for(header, DIST_HEADERS) {
      relPath = $${relative_path($$header, $$PRJ_BASE_DIR)}
      path = $${INCLUDE_PREFIX}/$${dirname(relPath)}
      eval(headers_$${path}.files += $$relPath)
      eval(headers_$${path}.path = $$path)
      eval(INSTALLS *= headers_$${path})
    }

    target = $$TARGET
    target.path = $$TARGET_PREFIX

    INSTALLS += target
}





