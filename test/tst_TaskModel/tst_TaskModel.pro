QT += testlib
QT += core gui widgets

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  \
    tst_taskmodel.cpp \
    ../TaskM/task.cpp \
    ../TaskM/taskmodel.cpp \
    ../TaskM/customdatamanager.cpp

HEADERS += \
    ../TaskM/task.h \
    ../TaskM/taskmodel.h \
    ../TaskM/customdatamanager.h

INCLUDEPATH += ../TaskM

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
