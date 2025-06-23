#include "task.h"
#include <QDateTime>
#include <QUuid>
#include <QDate> // Added for QDate::currentDate()

Task::Task()
    : m_title(""),
    m_projectType("Обычная задача"),
    m_startDateTime(QDateTime(QDate::currentDate(), QTime(0, 0))),
    m_endDateTime(QDateTime(QDate::currentDate(), QTime(0, 0))),
    m_creationDateTime(QDateTime::currentDateTime()),
    m_isProjectTask(false),
    m_status("Не начато"),
    m_description(""),
    m_priority("Средний"),
    m_wasModified(false),
    m_uid(QUuid::createUuid())
{
}

Task::Task(const QString &title, const QString &projectType, const QDateTime &start, const QDateTime &end, const QString &status, const QString &description)
    : m_title(title),
    m_projectType(projectType),
    m_startDateTime(start),
    m_endDateTime(end),
    m_creationDateTime(QDateTime::currentDateTime()),
    m_isProjectTask(true),
    m_status(status),
    m_description(description),
    m_priority("Средний"),
    m_wasModified(false),
    m_uid(QUuid::createUuid())
{
}

QString Task::formatDate(const Task &task)
{
    QDate today = QDate::currentDate();
    if (task.isProjectTask()) {
        QDateTime start = task.startDateTime();
        QDateTime end = task.endDateTime();
        QString dateText = (start.date() == today) ? "Сегодня" : start.date().toString("dd.MM.yyyy");
        return QString("%1 %2–%3")
            .arg(dateText)
            .arg(start.time().toString("HH:mm"))
            .arg(end.time().toString("HH:mm"));
    } else {
        QDateTime due = task.dueDateTime();
        if (due.date() == today) {
            return "Сегодня";
        }
        return due.date().toString("dd.MM.yyyy");
    }
}
