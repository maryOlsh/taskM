QT       += core gui widgets qml testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 debug
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -lgcov
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    task.cpp \
    taskmodel.cpp \
    taskdialog.cpp \
    taskdelegate.cpp \
    taskfilterproxymodel.cpp \
    taskscheduleoverlay.cpp \
    customdatamanager.cpp \
    namecolordialog.cpp \
    namedialog.cpp \


HEADERS += \
    mainwindow.h \
    task.h \
    taskmodel.h \
    taskdialog.h \
    taskdelegate.h \
    taskfilterproxymodel.h \
    taskscheduleoverlay.h \
    taskslot.h \
    customdatamanager.h \
    namecolordialog.h \
    namedialog.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    tst_test2.qml


