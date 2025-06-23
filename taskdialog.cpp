/**
 * @file taskdialog.cpp
 * @brief Реализация диалога создания и редактирования задач.
 */
#include "taskdialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QDateEdit>
#include <QTimeEdit>
#include "customdatamanager.h"

TaskDialog::TaskDialog(CustomDataManager *dataManager, QWidget *parent, const Task &task)
    : QDialog(parent), m_dataManager(dataManager), m_originalTask(task)
{
    setWindowTitle(task.title().isEmpty() ? "Новая задача" : "Редактирование задачи");
    setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Заголовок
    titleEdit = new QLineEdit(task.title(), this);
    titleEdit->setPlaceholderText("Введите заголовок задачи");
    mainLayout->addWidget(new QLabel("Заголовок:"));
    mainLayout->addWidget(titleEdit);

    // Описание
    descriptionEdit = new QTextEdit(this);
    descriptionEdit->setText(task.description());
    descriptionEdit->setPlaceholderText("Введите описание задачи");
    mainLayout->addWidget(new QLabel("Описание:"));
    mainLayout->addWidget(descriptionEdit);

    // Статус
    statusCombo = new QComboBox(this);
    mainLayout->addWidget(new QLabel("Статус:"));
    mainLayout->addWidget(statusCombo);

    // Приоритет
    priorityCombo = new QComboBox(this);
    mainLayout->addWidget(new QLabel("Приоритет:"));
    mainLayout->addWidget(priorityCombo);

    // Тип задачи
    projectTypeCombo = new QComboBox(this);
    mainLayout->addWidget(new QLabel("Проект:"));
    mainLayout->addWidget(projectTypeCombo);

    populateCombos();

    statusCombo->setCurrentText(task.status().isEmpty() ? "Не начато" : task.status());
    priorityCombo->setCurrentText(task.priority().isEmpty() ? "Средний" : task.priority());
    projectTypeCombo->setCurrentText(task.projectType());


    // Чекбокс "Задача по времени" (НЕ зависит от выбора проекта)
    isTimedTaskCheck = new QCheckBox("Задача по времени", this);
    mainLayout->addWidget(isTimedTaskCheck);

    // Дата
    dateEdit = new QDateEdit(this);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDate(task.startDateTime().isValid() ? task.startDateTime().date() : QDate::currentDate());
    mainLayout->addWidget(new QLabel("Дата:"));
    mainLayout->addWidget(dateEdit);

    // Время начала и окончания
    QHBoxLayout *timeLayout = new QHBoxLayout();
    startTimeEdit = new QTimeEdit(this);
    endTimeEdit = new QTimeEdit(this);
    startTimeEdit->setTime(task.startDateTime().isValid() ? task.startDateTime().time() : QTime(9,0));
    endTimeEdit->setTime(task.endDateTime().isValid() ? task.endDateTime().time() : QTime(10,0));
    startTimeEdit->setDisplayFormat("HH:mm");
    endTimeEdit->setDisplayFormat("HH:mm");
    // Округление времени до 10 минут при завершении редактирования
    connect(startTimeEdit, &QTimeEdit::editingFinished, this, [this]() {
        QTime t = startTimeEdit->time();
        int minutes = (t.minute() / 10) * 10;
        startTimeEdit->setTime(QTime(t.hour(), minutes));
    });
    connect(endTimeEdit, &QTimeEdit::editingFinished, this, [this]() {
        QTime t = endTimeEdit->time();
        int minutes = (t.minute() / 10) * 10;
        endTimeEdit->setTime(QTime(t.hour(), minutes));
    });
    timeLayout->addWidget(new QLabel("Начало:"));
    timeLayout->addWidget(startTimeEdit);
    timeLayout->addWidget(new QLabel("Окончание:"));
    timeLayout->addWidget(endTimeEdit);
    mainLayout->addLayout(timeLayout);

    // Кнопки
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TaskDialog::validateAndAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    // Сигнал для чекбокса
    connect(isTimedTaskCheck, &QCheckBox::toggled, this, &TaskDialog::onTimedTaskToggled);
    // Удаляем блокировку чекбокса по projectTypeCombo
    // Чекбокс всегда доступен
    // Инициализация видимости
    bool isTimed = task.isProjectTask();
    isTimedTaskCheck->setChecked(isTimed);
    onTimedTaskToggled(isTimed);
}

void TaskDialog::populateCombos()
{
    if (!m_dataManager) return;

    projectTypeCombo->clear();
    projectTypeCombo->addItems(m_dataManager->getProjects().keys());

    statusCombo->clear();
    QStringList statuses = m_dataManager->getStatuses();
    statuses.removeAll("Просрочено");
    statusCombo->addItems(statuses);

    priorityCombo->clear();
    priorityCombo->addItems(m_dataManager->getPriorities().keys());
}

void TaskDialog::onTimedTaskToggled(bool checked)
{
    startTimeEdit->setVisible(checked);
    endTimeEdit->setVisible(checked);
}

void TaskDialog::validateAndAccept()
{
    if (titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заголовок задачи не может быть пустым");
        return;
    }
    if (isTimedTaskCheck->isChecked()) {
        if (startTimeEdit->time() >= endTimeEdit->time()) {
            QMessageBox::warning(this, "Ошибка", "Время окончания должно быть позже времени начала");
            return;
        }
    }
    accept();
}

Task TaskDialog::getTask() const
{
    Task task = m_originalTask;
    task.setTitle(titleEdit->text().trimmed());
    task.setDescription(descriptionEdit->toPlainText().trimmed());
    task.setStatus(statusCombo->currentText());
    task.setPriority(priorityCombo->currentText());
    task.setProjectType(projectTypeCombo->currentText());
    // isProjectTask выставляется независимо от projectType
    task.setIsProjectTask(isTimedTaskCheck->isChecked());
    task.setWasModified(false); // Clear the flag on any manual edit
    if (task.isProjectTask()) {
        QDate date = dateEdit->date();
        QTime start = startTimeEdit->time();
        QTime end = endTimeEdit->time();
        task.setStartDateTime(QDateTime(date, start));
        task.setEndDateTime(QDateTime(date, end));
    } else {
        QDate date = dateEdit->date();
        task.setStartDateTime(QDateTime(date, QTime(0,0)));
        task.setEndDateTime(QDateTime(date, QTime(0,0)));
    }
    qDebug() << "TaskDialog::getTask returning:" << task.title() << task.projectType() << task.isProjectTask();
    return task;
}

void TaskDialog::setDueDateTime(const QDateTime &dateTime)
{
    dateEdit->setDate(dateTime.date());
    startTimeEdit->setTime(dateTime.time());
    endTimeEdit->setTime(dateTime.time().addSecs(3600));
}

int TaskDialog::exec()
{
    return QDialog::exec();
}
