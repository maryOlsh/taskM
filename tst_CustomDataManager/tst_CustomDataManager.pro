QT += testlib
QT += core gui widgets

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  \
    tst_customdatamanager.cpp \
    ../TaskM/customdatamanager.cpp

HEADERS += \
    ../TaskM/customdatamanager.h

INCLUDEPATH += ../TaskM 