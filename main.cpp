/**
 * @file main.cpp
 * @brief Точка входа в приложение TaskM.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<Task>();
    MainWindow w;
    w.show();
    return a.exec();
}
