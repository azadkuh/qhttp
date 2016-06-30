#-------------------------------------------------
#
# Project created by QtCreator 2017-01-20T01:28:47
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = uStaticServer
TEMPLATE = app


PRJDIR       = ../..
include($$PRJDIR/commondir.pri)

SOURCES += main.cpp\
        mainwindow.cpp

LIBS        += -lqhttp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
