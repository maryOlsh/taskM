/**
 * @file task.h
 * @brief Класс Task — описание задачи.
 */

#ifndef TASK_H
#define TASK_H

#include <QDateTime>
#include <QDebug>
#include <QUuid>

/**
 * @class Task
 * @brief Класс, представляющий задачу (проектную или обычную).
 */
class Task {
public:
    Task();
    Task(const QString &title, const QString &projectType, const QDateTime &start, const QDateTime &end = QDateTime(), const QString &status = "", const QString &description = "");

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    QString projectType() const { return m_projectType; }
    void setProjectType(const QString &type) { m_projectType = type; }


    QDateTime startDateTime() const { return m_startDateTime; }
    void setStartDateTime(const QDateTime &dt) { m_startDateTime = dt; }

    QDateTime endDateTime() const { return m_endDateTime; }
    void setEndDateTime(const QDateTime &dt) { m_endDateTime = dt; }

    QDateTime creationDateTime() const { return m_creationDateTime; }
    void setCreationDateTime(const QDateTime &dt) { m_creationDateTime = dt; }

    QDateTime dueDateTime() const { return m_isProjectTask ? m_endDateTime : m_startDateTime; }
    void setDueDateTime(const QDateTime &dt) {
        if (m_isProjectTask)
            m_endDateTime = dt;
        else
            m_startDateTime = dt;
    }

    bool isProjectTask() const { return m_isProjectTask; }
    void setIsProjectTask(bool value) { m_isProjectTask = value; }

    QString status() const { return m_status; }
    void setStatus(const QString &status) { m_status = status; }

    QString description() const { return m_description; }
    void setDescription(const QString &description) { m_description = description; }

    QString priority() const { return m_priority; }
    void setPriority(const QString &priority) { m_priority = priority; }

    QUuid uid() const { return m_uid; }
    void setUid(const QUuid &id) { m_uid = id; }

    bool wasModified() const { return m_wasModified; }
    void setWasModified(bool modified) { m_wasModified = modified; }

    static QString formatDate(const Task& task);

private:
    QString m_title;
    QString m_projectType;
    QDateTime m_startDateTime;
    QDateTime m_endDateTime;
    QDateTime m_creationDateTime;
    bool m_isProjectTask = false;
    QString m_status;
    QString m_description;
    QString m_priority;
    bool m_wasModified = false;
    QUuid m_uid;
};



#include <QMetaType>
Q_DECLARE_METATYPE(Task)

#endif // TASK_
