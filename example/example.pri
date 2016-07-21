equals(ENABLE_QHTTP_CLIENT, "1") {
    DEFINES *= QHTTP_HAS_CLIENT
}

CONFIG      += c++11 c++14

INCLUDEPATH += $$PWD/../include

LIBS        += $$OUT_PWD/../../src/libqhttp.a
