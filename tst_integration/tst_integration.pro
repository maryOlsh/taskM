QT += testlib core widgets
CONFIG += console
SOURCES += tst_integration.cpp \
           ../TaskM/taskmodel.cpp \
           ../TaskM/taskfilterproxymodel.cpp \
           ../TaskM/task.cpp \
           ../TaskM/customdatamanager.cpp

HEADERS += ../TaskM/taskmodel.h \
           ../TaskM/taskfilterproxymodel.h \
           ../TaskM/customdatamanager.h \
           ../TaskM/task.h

INCLUDEPATH += ../TaskM
