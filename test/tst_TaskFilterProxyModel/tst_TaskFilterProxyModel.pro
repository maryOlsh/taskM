QT += testlib
QT += core gui widgets
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -lgcov
CONFIG += debug
CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  \
    tst_taskfilterproxymodel.cpp \
    ../../taskfilterproxymodel.cpp \
    ../../taskmodel.cpp \
    ../../task.cpp \
    ../../customdatamanager.cpp

HEADERS += \
    ../../taskfilterproxymodel.h \
    ../../taskmodel.h \
    ../../task.h \
    ../../customdatamanager.h

INCLUDEPATH += ../../
