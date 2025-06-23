// tst_task.cpp
#include <QtTest>
#include "task.h"

class TaskTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase(); // будет выполняться перед всеми тестами
    void cleanupTestCase(); // будет выполняться после всех тестов
    void init(); // будет выполняться перед каждым тестом
    void cleanup(); // будет выполняться после каждого теста

    // Тесты
    void defaultValues();
    void settersAndGetters();
    void dateTimeTests();
    void formatDateTests();
    void uidTests();
};

void TaskTest::initTestCase()
{
    qDebug() << "Starting TaskTest";
}

void TaskTest::cleanupTestCase()
{
    qDebug() << "Finishing TaskTest";
}

void TaskTest::init()
{
    // Подготовка каждого теста
}

void TaskTest::cleanup()
{
    // Очистка после каждого теста
}

void TaskTest::defaultValues()
{
    Task t;
    QVERIFY(t.title().isEmpty());
    QCOMPARE(t.projectType(), QString("Обычная задача"));
    QCOMPARE(t.status(), QString("Не начато"));
    QCOMPARE(t.priority(), QString("Средний"));
    QVERIFY(!t.isProjectTask());
    QVERIFY(!t.wasModified());
    QVERIFY(!t.uid().isNull());
}

void TaskTest::settersAndGetters()
{
    Task t;
    QString testTitle = "Test Task";
    QString testProject = "Work";
    QString testStatus = "Выполнено";
    QString testPriority = "Высокий";
    QString testDescription = "Test Description";

    t.setTitle(testTitle);
    t.setProjectType(testProject);
    t.setStatus(testStatus);
    t.setPriority(testPriority);
    t.setDescription(testDescription);
    t.setIsProjectTask(true);

    QCOMPARE(t.title(), testTitle);
    QCOMPARE(t.projectType(), testProject);
    QCOMPARE(t.status(), testStatus);
    QCOMPARE(t.priority(), testPriority);
    QCOMPARE(t.description(), testDescription);
    QVERIFY(t.isProjectTask());
}

void TaskTest::dateTimeTests()
{
    Task t;
    QDateTime now = QDateTime::currentDateTime();
    QDateTime later = now.addSecs(3600); // +1 час

    t.setStartDateTime(now);
    t.setEndDateTime(later);
    t.setIsProjectTask(true);

    QCOMPARE(t.startDateTime(), now);
    QCOMPARE(t.endDateTime(), later);
    QCOMPARE(t.dueDateTime(), later); // для проектной задачи dueDateTime = endDateTime

    // Тест для обычной задачи
    t.setIsProjectTask(false);
    QCOMPARE(t.dueDateTime(), now); // для обычной задачи dueDateTime = startDateTime
}

void TaskTest::formatDateTests()
{
    QDate today = QDate::currentDate();
    QTime time(10, 0);
    QDateTime dateTime(today, time);

    // Тест для проектной задачи
    Task projectTask;
    projectTask.setIsProjectTask(true);
    projectTask.setStartDateTime(dateTime);
    projectTask.setEndDateTime(dateTime.addSecs(3600));

    QString expectedFormat = QString("Сегодня %1–%2")
                                 .arg(time.toString("HH:mm"))
                                 .arg(time.addSecs(3600).toString("HH:mm"));

    QCOMPARE(Task::formatDate(projectTask), expectedFormat);

    // Тест для обычной задачи
    Task regularTask;
    regularTask.setStartDateTime(dateTime);
    QCOMPARE(Task::formatDate(regularTask), "Сегодня");
}

void TaskTest::uidTests()
{
    Task t1;
    Task t2;

    // Проверяем, что UID генерируются и уникальны
    QVERIFY(!t1.uid().isNull());
    QVERIFY(!t2.uid().isNull());
    QVERIFY(t1.uid() != t2.uid());

    // Проверяем установку UID
    QUuid testUid = QUuid::createUuid();
    t1.setUid(testUid);
    QCOMPARE(t1.uid(), testUid);
}

QTEST_APPLESS_MAIN(TaskTest)
#include "tst_task.moc"
