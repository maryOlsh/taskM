QT += testlib core
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -lgcov
CONFIG += debug
CONFIG += console c++17 cmdline
SOURCES += ../../task.cpp \
           tst_task.cpp
INCLUDEPATH += ../../



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
