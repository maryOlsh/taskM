QT += testlib core widgets
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -lgcov
CONFIG += debug
CONFIG += console
SOURCES += tst_integration.cpp \
           ../../taskmodel.cpp \
           ../../taskfilterproxymodel.cpp \
           ../../task.cpp \
           ../../customdatamanager.cpp

HEADERS += ../../taskmodel.h \
           ../../taskfilterproxymodel.h \
           ../../customdatamanager.h \
           ../../task.h

INCLUDEPATH += ../../
