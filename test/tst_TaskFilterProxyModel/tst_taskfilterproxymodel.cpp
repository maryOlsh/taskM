#include <QtTest>
#include <QCoreApplication>
#include "../TaskM/taskfilterproxymodel.h"
#include "../TaskM/taskmodel.h"
#include "../TaskM/task.h"
#include "../TaskM/customdatamanager.h"
#include <QStandardPaths>
#include <QDir>

class TaskFilterProxyModelTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        m_tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/TaskFilterProxyModelTest";
        QDir dir(m_tempPath);
        if (dir.exists()) {
            dir.removeRecursively();
        }
        dir.mkpath(".");
        qDebug() << "Test directory:" << m_tempPath;
    }

    void cleanupTestCase() {
        QDir dir(m_tempPath);
        if (dir.exists()) {
            dir.removeRecursively();
        }
    }

    void init() {
        m_dataManager = new CustomDataManager(this);
        m_sourceModel = new TaskModel(nullptr, m_dataManager);
        m_proxyModel = new TaskFilterProxyModel(this);
        m_proxyModel->setSourceModel(m_sourceModel);

        // Добавляем тестовые задачи
        setupTestTasks();
    }

    void cleanup() {
        delete m_proxyModel;
        delete m_sourceModel;
        delete m_dataManager;
        m_proxyModel = nullptr;
        m_sourceModel = nullptr;
        m_dataManager = nullptr;
    }

    // Тест фильтрации по дате
    void testDateFilter() {
        QDate testDate(2025, 6, 23); // фиксированная дата
        m_proxyModel->setFilterDate(testDate);

        // Отладочный вывод: все задачи в исходной модели
        qDebug() << "\n=== DEBUG: All tasks in source model ===";
        for (int row = 0; row < m_sourceModel->rowCount(); ++row) {
            QModelIndex idx = m_sourceModel->index(row, 0);
            Task t = m_sourceModel->getTask(idx.row());
            qDebug() << "title:" << t.title()
                     << ", isProjectTask:" << t.isProjectTask()
                     << ", start:" << t.startDateTime()
                     << ", end:" << t.endDateTime()
                     << ", due:" << t.dueDateTime()
                     << ", status:" << t.status();
        }
        qDebug() << "Filter date:" << testDate;
        qDebug() << "Proxy rowCount:" << m_proxyModel->rowCount();

        // Должны быть видны задачи на текущую дату (и с временем, и без)
        QCOMPARE(m_proxyModel->rowCount(), 2);

        // Проверяем фильтрацию на другую дату
        m_proxyModel->setFilterDate(testDate.addDays(1));
        QCOMPARE(m_proxyModel->rowCount(), 0);

        // Сброс фильтра даты
        m_proxyModel->setFilterDate(QDate());
        QCOMPARE(m_proxyModel->rowCount(), 2);
    }

    // Тест фильтрации по проекту
    void testProjectFilter() {
        m_proxyModel->setFilterDate(QDate());
        m_proxyModel->setFilterProjectType("Проект A");
        QCOMPARE(m_proxyModel->rowCount(), 1); // только "живые" задачи

        m_proxyModel->setFilterProjectType("Проект B");
        QCOMPARE(m_proxyModel->rowCount(), 1);

        m_proxyModel->setFilterProjectType("");
        QCOMPARE(m_proxyModel->rowCount(), 2);
    }

    // Тест фильтрации по заголовку
    void testTitleFilter() {
        m_proxyModel->setFilterDate(QDate());
        m_proxyModel->setFilterTitle("Задача 1");
        QCOMPARE(m_proxyModel->rowCount(), 1);

        m_proxyModel->setFilterTitle("Задача");
        QCOMPARE(m_proxyModel->rowCount(), 2);

        m_proxyModel->setFilterTitle("");
        QCOMPARE(m_proxyModel->rowCount(), 2);
    }

    // Тест фильтрации по статусу
    void testStatusFilter() {
        m_proxyModel->setFilterDate(QDate());
        m_proxyModel->setFilterStatus("В процессе");
        QCOMPARE(m_proxyModel->rowCount(), 1);

        m_proxyModel->setFilterStatus("Выполнено");
        QCOMPARE(m_proxyModel->rowCount(), 1);

        m_proxyModel->setFilterStatus("");
        QCOMPARE(m_proxyModel->rowCount(), 2);
    }

    // Тест фильтрации по приоритету
    void testPriorityFilter() {
        m_proxyModel->setFilterDate(QDate());
        m_proxyModel->setFilterPriority("Высокий");
        QCOMPARE(m_proxyModel->rowCount(), 1);

        m_proxyModel->setFilterPriority("Низкий");
        QCOMPARE(m_proxyModel->rowCount(), 0); // нет "живых" задач с этим приоритетом

        m_proxyModel->setFilterPriority("");
        QCOMPARE(m_proxyModel->rowCount(), 2);
    }

    // Тест фильтрации по типу дедлайна
    void testDeadlineTypeFilter() {
        m_proxyModel->setFilterDate(QDate());
        // Все задачи (живые)
        m_proxyModel->setFilterDeadlineType(0);
        QCOMPARE(m_proxyModel->rowCount(), 2);

        // Только предстоящие
        m_proxyModel->setFilterDeadlineType(1);
        int upcomingCount = m_proxyModel->rowCount();
        QVERIFY(upcomingCount > 0);

        // Только просроченные
        m_proxyModel->setFilterDeadlineType(2);
        QCOMPARE(m_proxyModel->rowCount(), 1);

        // Только выполненные
        m_proxyModel->setFilterDeadlineType(3);
        QCOMPARE(m_proxyModel->rowCount(), 1);
    }

    // Тест фильтрации по типу задачи (с временем/без времени)
    void testProjectTaskFilter() {
        m_proxyModel->setFilterDate(QDate());
        // Все задачи (живые)
        m_proxyModel->setFilterIsProjectTask(-1);
        QCOMPARE(m_proxyModel->rowCount(), 2);

        // Только задачи без времени
        m_proxyModel->setFilterIsProjectTask(0);
        QCOMPARE(m_proxyModel->rowCount(), 1);

        // Только задачи по времени
        m_proxyModel->setFilterIsProjectTask(1);
        QCOMPARE(m_proxyModel->rowCount(), 1);
    }

    // Тест комбинированной фильтрации
    void testCombinedFilters() {
        m_proxyModel->setFilterDate(QDate());
        m_proxyModel->setFilterProjectType("Проект A");
        m_proxyModel->setFilterStatus("В процессе");
        QCOMPARE(m_proxyModel->rowCount(), 1);

        m_proxyModel->setFilterPriority("Высокий");
        QCOMPARE(m_proxyModel->rowCount(), 1);

        m_proxyModel->setFilterTitle("Задача 1");
        QCOMPARE(m_proxyModel->rowCount(), 1);
    }

    // Тест игнорирования дедлайна для оверлея
    void testIgnoreDeadlineFilter() {
        QDate testDate(2025, 6, 23);
        m_proxyModel->setFilterDate(testDate);
        
        // Проверяем только задачи по времени на текущую дату
        int count = 0;
        for(int i = 0; i < m_sourceModel->rowCount(); ++i) {
            if(m_proxyModel->filterAcceptsRow_IgnoreDeadline(i, QModelIndex())) {
                count++;
            }
        }
        QCOMPARE(count, 1);
    }

private:
    QString m_tempPath;
    CustomDataManager* m_dataManager;
    TaskModel* m_sourceModel;
    TaskFilterProxyModel* m_proxyModel;

    void setupTestTasks() {
        QDate baseDate(2025, 6, 23);
        QTime baseTime(12, 0);
        QDateTime baseDateTime(baseDate, baseTime);

        // Задача 1: текущая дата, с временем
        Task task1;
        task1.setTitle("Задача 1");
        task1.setProjectType("Проект A");
        task1.setStatus("В процессе");
        task1.setPriority("Высокий");
        task1.setIsProjectTask(true);
        task1.setStartDateTime(baseDateTime);
        task1.setEndDateTime(baseDateTime.addSecs(3600));
        m_sourceModel->addTask(task1);

        // Задача 2: текущая дата, без времени (дата 2025-06-23 00:00)
        Task task2;
        task2.setTitle("Задача 2");
        task2.setProjectType("Проект B");
        task2.setStatus("Не начато");
        task2.setPriority("Средний");
        task2.setIsProjectTask(false);
        QDateTime task2DateTime(baseDate, QTime(0, 0));
        task2.setStartDateTime(task2DateTime);
        task2.setEndDateTime(task2DateTime);
        m_sourceModel->addTask(task2);

        // Задача 3: следующая дата, с временем
        Task task3;
        task3.setTitle("Задача 3");
        task3.setProjectType("Проект A");
        task3.setStatus("Выполнено");
        task3.setPriority("Низкий");
        task3.setIsProjectTask(true);
        task3.setStartDateTime(baseDateTime.addDays(1));
        task3.setEndDateTime(baseDateTime.addDays(1).addSecs(3600));
        m_sourceModel->addTask(task3);

        // Задача 4: прошлая дата, без времени (дата 2025-06-22 00:00)
        Task task4;
        task4.setTitle("Задача 4");
        task4.setProjectType("Обычная задача");
        task4.setStatus("Просрочено");
        task4.setPriority("Средний");
        task4.setIsProjectTask(false);
        QDateTime task4DateTime(baseDate.addDays(-1), QTime(0, 0));
        task4.setStartDateTime(task4DateTime);
        task4.setEndDateTime(task4DateTime);
        m_sourceModel->addTask(task4);
    }
};

QTEST_MAIN(TaskFilterProxyModelTest)
#include "tst_taskfilterproxymodel.moc"
