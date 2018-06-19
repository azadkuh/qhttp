QT          += core network
QT          -= gui
CONFIG      += console
osx:CONFIG  -= app_bundle

TEMPLATE     = app

PRJDIR       = ../..
include(../qmake/configs.pri)

LIBS      += -lqhttp

