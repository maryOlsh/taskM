QT += testlib
QT += core gui widgets
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -lgcov
CONFIG += qt console warn_on depend_includepath testcase debug
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  \
    tst_customdatamanager.cpp \
    ../../customdatamanager.cpp

HEADERS += \
    ../../customdatamanager.h

INCLUDEPATH += ../../
