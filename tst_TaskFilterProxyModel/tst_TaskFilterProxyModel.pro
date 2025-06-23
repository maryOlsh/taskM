QT += testlib
QT += core gui widgets

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  \
    tst_taskfilterproxymodel.cpp \
    ../TaskM/taskfilterproxymodel.cpp \
    ../TaskM/taskmodel.cpp \
    ../TaskM/task.cpp \
    ../TaskM/customdatamanager.cpp

HEADERS += \
    ../TaskM/taskfilterproxymodel.h \
    ../TaskM/taskmodel.h \
    ../TaskM/task.h \
    ../TaskM/customdatamanager.h

INCLUDEPATH += ../TaskM 