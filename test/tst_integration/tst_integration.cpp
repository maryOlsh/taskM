#include <QtTest>
#include "taskmodel.h"
#include "taskfilterproxymodel.h"
#include "task.h"
#include <QDebug>

class IntegrationTest : public QObject {
    Q_OBJECT
private slots:
    void addAndFilterByProject();
    void changeStatusAndFilter();
    void removeTaskAndCheck();
    void combinedFilters();
    void filterByDate();
};

void IntegrationTest::addAndFilterByProject() {
    TaskModel model;
    Task t1("Task 1", "Work", QDateTime(QDate(2025,6,23), QTime(10,0)), QDateTime(QDate(2025,6,23), QTime(11,0)), "Не начато", "");
    Task t2("Task 2", "Home", QDateTime(QDate(2025,6,24), QTime(12,0)), QDateTime(QDate(2025,6,24), QTime(13,0)), "В процессе", "");
    model.addTask(t1);
    model.addTask(t2);
    TaskFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    proxy.setFilterProjectType("Work");
    QCOMPARE(proxy.rowCount(), 1);
    proxy.setFilterProjectType("Home");
    QCOMPARE(proxy.rowCount(), 1);
    proxy.setFilterProjectType("");
    QCOMPARE(proxy.rowCount(), 2);
}

void IntegrationTest::changeStatusAndFilter() {
    TaskModel model;
    Task t1("Task 1", "Work", QDateTime(QDate(2025,6,23), QTime(10,0)), QDateTime(QDate(2025,6,23), QTime(11,0)), "Не начато", "");
    model.addTask(t1);
    TaskFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    proxy.setFilterStatus("Не начато");
    QCOMPARE(proxy.rowCount(), 1);
    // Меняем статус на "Выполнено"
    Task updated = model.getTask(0);
    updated.setStatus("Выполнено");
    model.updateTask(0, updated);
    // По умолчанию proxy скрывает "Выполнено"
    QCOMPARE(proxy.rowCount(), 0);
}

void IntegrationTest::removeTaskAndCheck() {
    TaskModel model;
    Task t1("Task 1", "Work", QDateTime(QDate(2025,6,23), QTime(10,0)), QDateTime(QDate(2025,6,23), QTime(11,0)), "Не начато", "");
    Task t2("Task 2", "Work", QDateTime(QDate(2025,6,24), QTime(12,0)), QDateTime(QDate(2025,6,24), QTime(13,0)), "В процессе", "");
    model.addTask(t1);
    model.addTask(t2);
    TaskFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    proxy.setFilterProjectType("Work");
    QCOMPARE(proxy.rowCount(), 2);
    model.removeTask(0);
    QCOMPARE(proxy.rowCount(), 1);
    model.removeTask(0);
    QCOMPARE(proxy.rowCount(), 0);
}

void IntegrationTest::combinedFilters() {
    TaskModel model;
    Task t1("Task 1", "Work", QDateTime(QDate(2025,6,23), QTime(10,0)), QDateTime(QDate(2025,6,23), QTime(11,0)), "", "");
    t1.setStatus("Не начато");
    t1.setPriority("Высокий");
    Task t2("Task 2", "Work", QDateTime(QDate(2025,6,24), QTime(12,0)), QDateTime(QDate(2025,6,24), QTime(13,0)), "", "");
    t2.setStatus("В процессе");
    t2.setPriority("Средний");
    Task t3("Task 3", "Home", QDateTime(QDate(2025,6,23), QTime(14,0)), QDateTime(QDate(2025,6,23), QTime(15,0)), "", "");
    t3.setStatus("Не начато");
    t3.setPriority("Высокий");
    model.addTask(t1);
    model.addTask(t2);
    model.addTask(t3);
    TaskFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    proxy.setFilterProjectType("Work");
    proxy.setFilterStatus("Не начато");
    proxy.setFilterPriority("Высокий");
    // Отладочный вывод
    qDebug() << "\n=== DEBUG: All tasks in source model ===";
    for (int row = 0; row < model.rowCount(); ++row) {
        Task t = model.getTask(row);
        qDebug() << "title:" << t.title()
                 << ", project:" << t.projectType()
                 << ", status:" << t.status()
                 << ", priority:" << t.priority();
    }
    qDebug() << "Proxy rowCount:" << proxy.rowCount();
    QCOMPARE(proxy.rowCount(), 1); // Только t1
    proxy.setFilterProjectType("");
    QCOMPARE(proxy.rowCount(), 2); // t1 и t3
}

void IntegrationTest::filterByDate() {
    TaskModel model;
    // Задача с временем
    Task t1("Task 1", "Work", QDateTime(QDate(2025,6,23), QTime(10,0)), QDateTime(QDate(2025,6,23), QTime(11,0)), "Не начато", "");
    // Обычная задача (без времени)
    Task t2;
    t2.setTitle("Task 2");
    t2.setProjectType("Work");
    t2.setStatus("Не начато");
    t2.setIsProjectTask(false);
    t2.setStartDateTime(QDateTime(QDate(2025,6,23), QTime(0,0)));
    t2.setEndDateTime(QDateTime(QDate(2025,6,23), QTime(0,0)));
    model.addTask(t1);
    model.addTask(t2);
    TaskFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    proxy.setFilterDate(QDate(2025,6,23));
    QCOMPARE(proxy.rowCount(), 2); // обе задачи на эту дату
    proxy.setFilterDate(QDate(2025,6,24));
    QCOMPARE(proxy.rowCount(), 0); // нет задач на эту дату
}

QTEST_APPLESS_MAIN(IntegrationTest)
#include "tst_integration.moc" 
