QT += testlib
QT += core gui widgets
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -lgcov
CONFIG += debug
CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  \
    tst_taskmodel.cpp \
    ../../task.cpp \
    ../../taskmodel.cpp \
    ../../customdatamanager.cpp

HEADERS += \
    ../../task.h \
    ../../taskmodel.h \
    ../../customdatamanager.h

INCLUDEPATH += ../../

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
