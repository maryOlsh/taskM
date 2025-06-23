QT += testlib core
CONFIG += console c++17 cmdline
SOURCES += ../TaskM/task.cpp \
           tst_task.cpp
INCLUDEPATH += ../TaskM



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
