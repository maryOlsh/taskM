#include <QtTest>
#include <QCoreApplication>
#include "../../taskmodel.h"
#include "../../task.h"
#include "../../customdatamanager.h"
#include <QStandardPaths>
#include <QDir>

class TaskModelTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        // Set up a temporary directory for test files
        m_tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/TaskModelTest";
        QDir dir(m_tempPath);
        if (dir.exists()) {
            dir.removeRecursively();
        }
        dir.mkpath(".");
        qDebug() << "Test directory:" << m_tempPath;
    }

    void cleanupTestCase() {
        // Clean up the temporary directory
        QDir dir(m_tempPath);
        if (dir.exists()) {
            dir.removeRecursively();
        }
    }

    void init() {
        // Set up before each test
    }

    void cleanup() {
        // Clean up after each test
    }

    // Test cases
    void testInitialState() {
        TaskModel model(nullptr);
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.columnCount(), TaskModel::ColumnCount);
    }

    void testAddTask() {
        TaskModel model(nullptr);
        Task task = createTestTask();
        model.addTask(task);
        QCOMPARE(model.rowCount(), 1);
        Task retrievedTask = model.getTask(0);
        QCOMPARE(retrievedTask.title(), task.title());
        QCOMPARE(retrievedTask.projectType(), task.projectType());
    }

    void testRemoveTask() {
        TaskModel model(nullptr);
        Task task = createTestTask();
        model.addTask(task);
        QCOMPARE(model.rowCount(), 1);
        model.removeTask(0);
        QCOMPARE(model.rowCount(), 0);
    }

    void testUpdateTask() {
        TaskModel model(nullptr);
        Task task = createTestTask();
        model.addTask(task);
        
        Task updatedTask = task;
        updatedTask.setTitle("Updated Title");
        model.updateTask(0, updatedTask);
        
        Task retrievedTask = model.getTask(0);
        QCOMPARE(retrievedTask.title(), QString("Updated Title"));
    }

    void testData() {
        TaskModel model(nullptr);
        Task task = createTestTask();
        model.addTask(task);
        
        QModelIndex index = model.index(0, TaskModel::TitleColumn);
        QCOMPARE(model.data(index, Qt::DisplayRole).toString(), task.title());
        
        index = model.index(0, TaskModel::ProjectColumn);
        QCOMPARE(model.data(index, Qt::DisplayRole).toString(), task.projectType());
    }

    void testHeaderData() {
        TaskModel model(nullptr);
        QCOMPARE(model.headerData(TaskModel::TitleColumn, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Название"));
        QCOMPARE(model.headerData(TaskModel::DateColumn, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Дата"));
        QCOMPARE(model.headerData(TaskModel::ProjectColumn, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Проект"));
    }

    void testRoleNames() {
        TaskModel model(nullptr);
        QHash<int, QByteArray> roles = model.roleNames();
        QVERIFY(roles.contains(TaskModel::TitleRole));
        QVERIFY(roles.contains(TaskModel::ProjectRole));
        QVERIFY(roles.contains(TaskModel::DueDateRole));
    }

    void testFindTask() {
        TaskModel model(nullptr);
        Task task = createTestTask();
        model.addTask(task);
        
        int index = model.findTask(task.uid());
        QCOMPARE(index, 0);
        
        index = model.findTask(QUuid::createUuid());
        QCOMPARE(index, -1);
    }

    void testFindTasksByProject() {
        TaskModel model(nullptr);
        Task task1 = createTestTask("Task 1");
        task1.setProjectType("Project A");
        Task task2 = createTestTask("Task 2");
        task2.setProjectType("Project B");
        Task task3 = createTestTask("Task 3");
        task3.setProjectType("Project A");
        
        model.addTask(task1);
        model.addTask(task2);
        model.addTask(task3);
        
        QVector<int> projectATasks = model.findTasksUsingProject("Project A");
        QCOMPARE(projectATasks.size(), 2);
        QVERIFY(projectATasks.contains(0));
        QVERIFY(projectATasks.contains(2));
    }

    void testFindTasksByStatus() {
        TaskModel model(nullptr);
        Task task1 = createTestTask("Task 1");
        task1.setStatus("В процессе");
        Task task2 = createTestTask("Task 2");
        task2.setStatus("Выполнено");
        
        model.addTask(task1);
        model.addTask(task2);
        
        QVector<int> inProgressTasks = model.findTasksUsingStatus("В процессе");
        QCOMPARE(inProgressTasks.size(), 1);
        QCOMPARE(inProgressTasks[0], 0);
    }

    void testFindTasksByPriority() {
        TaskModel model(nullptr);
        Task task1 = createTestTask("Task 1");
        task1.setPriority("Высокий");
        Task task2 = createTestTask("Task 2");
        task2.setPriority("Низкий");
        
        model.addTask(task1);
        model.addTask(task2);
        
        QVector<int> highPriorityTasks = model.findTasksUsingPriority("Высокий");
        QCOMPARE(highPriorityTasks.size(), 1);
        QCOMPARE(highPriorityTasks[0], 0);
    }

    void testReplaceProjectInTasks() {
        TaskModel model(nullptr);
        Task task1 = createTestTask("Task 1");
        task1.setProjectType("Old Project");
        Task task2 = createTestTask("Task 2");
        task2.setProjectType("Old Project");
        
        model.addTask(task1);
        model.addTask(task2);
        
        QVector<int> indices = {0, 1};
        model.replaceProjectInTasks(indices, "New Project");
        
        QCOMPARE(model.getTask(0).projectType(), QString("New Project"));
        QCOMPARE(model.getTask(1).projectType(), QString("New Project"));
    }

    void testReplaceStatusInTasks() {
        TaskModel model(nullptr);
        Task task1 = createTestTask("Task 1");
        task1.setStatus("Не начато");
        Task task2 = createTestTask("Task 2");
        task2.setStatus("Не начато");
        
        model.addTask(task1);
        model.addTask(task2);
        
        QVector<int> indices = {0, 1};
        model.replaceStatusInTasks(indices, "В процессе");
        
        QCOMPARE(model.getTask(0).status(), QString("В процессе"));
        QCOMPARE(model.getTask(1).status(), QString("В процессе"));
    }

    void testReplacePriorityInTasks() {
        TaskModel model(nullptr);
        Task task1 = createTestTask("Task 1");
        task1.setPriority("Низкий");
        Task task2 = createTestTask("Task 2");
        task2.setPriority("Низкий");
        
        model.addTask(task1);
        model.addTask(task2);
        
        QVector<int> indices = {0, 1};
        model.replacePriorityInTasks(indices, "Высокий");
        
        QCOMPARE(model.getTask(0).priority(), QString("Высокий"));
        QCOMPARE(model.getTask(1).priority(), QString("Высокий"));
    }

    void testSaveLoad() {
        QString testFile = m_tempPath + "/test_tasks.json";
        qputenv("QT_STANDARD_PATHS_OVERRIDE", m_tempPath.toUtf8());
        
        // Create and save tasks
        {
            TaskModel model(nullptr);
            Task task1 = createTestTask("Task 1");
            Task task2 = createTestTask("Task 2");
            model.addTask(task1);
            model.addTask(task2);
            QVERIFY(model.saveTasks());
        }
        
        // Load tasks in a new model
        {
            TaskModel model(nullptr);
            QVERIFY(model.loadTasks());
            QCOMPARE(model.rowCount(), 2);
            QCOMPARE(model.getTask(0).title(), QString("Task 1"));
            QCOMPARE(model.getTask(1).title(), QString("Task 2"));
        }
    }

private:
    QString m_tempPath;

    Task createTestTask(const QString& title = "Test Task") {
        Task task;
        task.setTitle(title);
        task.setProjectType("Test Project");
        task.setStatus("Не начато");
        task.setPriority("Средний");
        task.setDescription("Test Description");
        task.setStartDateTime(QDateTime::currentDateTime());
        task.setEndDateTime(QDateTime::currentDateTime().addSecs(3600));
        return task;
    }
};

QTEST_MAIN(TaskModelTest)
#include "tst_taskmodel.moc"
