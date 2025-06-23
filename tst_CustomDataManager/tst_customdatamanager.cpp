#include <QtTest>
#include <QCoreApplication>
#include "../TaskM/customdatamanager.h"
#include <QStandardPaths>
#include <QDir>
#include <QColor>

class CustomDataManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        // Set up a temporary directory for test files
        m_tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/CustomDataManagerTest";
        QDir dir(m_tempPath);
        if (dir.exists()) {
            dir.removeRecursively();
        }
        dir.mkpath(".");
        qDebug() << "Test directory:" << m_tempPath;
        
        // Set the temporary path for the test
        qputenv("QT_STANDARD_PATHS_OVERRIDE", m_tempPath.toUtf8());
    }

    void cleanupTestCase() {
        // Clean up the temporary directory
        QDir dir(m_tempPath);
        if (dir.exists()) {
            dir.removeRecursively();
        }
    }

    void init() {
        // Create a fresh instance before each test
        m_manager = new CustomDataManager(this);
    }

    void cleanup() {
        // Clean up after each test
        delete m_manager;
        m_manager = nullptr;
    }

    // Test default initialization
    void testDefaultInitialization() {
        // Test default projects
        auto projects = m_manager->getProjects();
        QVERIFY(projects.contains("Работа"));
        QVERIFY(projects.contains("Учёба"));
        QVERIFY(projects.contains("Дом"));
        QVERIFY(projects.contains("Обычная задача"));

        // Test default statuses
        auto statuses = m_manager->getStatuses();
        QVERIFY(statuses.contains("Не начато"));
        QVERIFY(statuses.contains("В процессе"));
        QVERIFY(statuses.contains("Выполнено"));
        QVERIFY(statuses.contains("Отложено"));
        QVERIFY(statuses.contains("Просрочено"));

        // Test default priorities
        auto priorities = m_manager->getPriorities();
        QVERIFY(priorities.contains("Низкий"));
        QVERIFY(priorities.contains("Средний"));
        QVERIFY(priorities.contains("Высокий"));
    }

    // Test project management
    void testProjectManagement() {
        // Add new project
        QVERIFY(m_manager->addProject("Test Project", QColor(Qt::red)));
        QVERIFY(m_manager->getProjects().contains("Test Project"));
        QCOMPARE(m_manager->getProjectColor("Test Project"), QColor(Qt::red));

        // Try to add duplicate project
        QVERIFY(!m_manager->addProject("Test Project", QColor(Qt::blue)));

        // Try to remove system project
        QVERIFY(!m_manager->removeProject("Обычная задача"));

        // Remove custom project
        QVERIFY(m_manager->removeProject("Test Project"));
        QVERIFY(!m_manager->getProjects().contains("Test Project"));
    }

    // Test status management
    void testStatusManagement() {
        // Add new status
        QVERIFY(m_manager->addStatus("Test Status"));
        QVERIFY(m_manager->getStatuses().contains("Test Status"));

        // Try to add duplicate status
        QVERIFY(!m_manager->addStatus("Test Status"));

        // Try to remove system status
        QVERIFY(!m_manager->removeStatus("Не начато"));

        // Remove custom status
        QVERIFY(m_manager->removeStatus("Test Status"));
        QVERIFY(!m_manager->getStatuses().contains("Test Status"));
    }

    // Test priority management
    void testPriorityManagement() {
        // Add new priority
        QVERIFY(m_manager->addPriority("Test Priority", QColor(Qt::blue)));
        QVERIFY(m_manager->getPriorities().contains("Test Priority"));
        QCOMPARE(m_manager->getPriorityColor("Test Priority"), QColor(Qt::blue));

        // Try to add duplicate priority
        QVERIFY(!m_manager->addPriority("Test Priority", QColor(Qt::red)));

        // Try to remove system priority
        QVERIFY(!m_manager->removePriority("Средний"));

        // Remove custom priority
        QVERIFY(m_manager->removePriority("Test Priority"));
        QVERIFY(!m_manager->getPriorities().contains("Test Priority"));
    }

    // Test color retrieval
    void testColorRetrieval() {
        // Test project color retrieval
        QVERIFY(m_manager->getProjectColor("Работа") != QColor(100, 100, 100)); // Not default color
        QCOMPARE(m_manager->getProjectColor("NonExistent"), QColor(100, 100, 100)); // Default color

        // Test priority color retrieval
        QVERIFY(m_manager->getPriorityColor("Высокий") != Qt::gray); // Not default color
        QCOMPARE(m_manager->getPriorityColor("NonExistent"), Qt::gray); // Default color
    }

    // Test save and load
    void testSaveLoad() {
        // Add custom data
        m_manager->addProject("Custom Project", QColor(Qt::cyan));
        m_manager->addStatus("Custom Status");
        m_manager->addPriority("Custom Priority", QColor(Qt::magenta));

        // Save data
        m_manager->saveData();

        // Create new instance and load data
        CustomDataManager* newManager = new CustomDataManager(this);
        
        // Verify loaded data
        QVERIFY(newManager->getProjects().contains("Custom Project"));
        QVERIFY(newManager->getStatuses().contains("Custom Status"));
        QVERIFY(newManager->getPriorities().contains("Custom Priority"));
        
        QCOMPARE(newManager->getProjectColor("Custom Project"), QColor(Qt::cyan));
        QCOMPARE(newManager->getPriorityColor("Custom Priority"), QColor(Qt::magenta));

        delete newManager;
    }

    // Test system item checks
    void testSystemItems() {
        QVERIFY(m_manager->isSystemProject("Обычная задача"));
        QVERIFY(!m_manager->isSystemProject("Custom Project"));

        QVERIFY(m_manager->isSystemStatus("Не начато"));
        QVERIFY(!m_manager->isSystemStatus("Custom Status"));

        QVERIFY(m_manager->isSystemPriority("Средний"));
        QVERIFY(!m_manager->isSystemPriority("Custom Priority"));
    }

private:
    QString m_tempPath;
    CustomDataManager* m_manager;
};

QTEST_MAIN(CustomDataManagerTest)
#include "tst_customdatamanager.moc" 