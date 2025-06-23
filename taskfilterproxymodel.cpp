/**
 * @file taskfilterproxymodel.cpp
 * @brief Реализация прокси-модели для фильтрации задач.
 */
#include "taskfilterproxymodel.h"
#include "taskmodel.h"
#include <QDebug>

TaskFilterProxyModel::TaskFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
    m_filterDate(QDate()),
    m_filterProjectType(""),
    m_filterTitle(""),
    m_filterStatus(""),
    m_filterPriority(""),
    m_filterDeadlineType(0),
    m_filterIsProjectTask(-1)
{
}

void TaskFilterProxyModel::setFilterDate(const QDate &date)
{
    if (m_filterDate != date) {
        m_filterDate = date;
        invalidateFilter();
    }
}

void TaskFilterProxyModel::setFilterProjectType(const QString &projectType)
{
    if (m_filterProjectType != projectType) {
        m_filterProjectType = projectType;
        invalidateFilter();
    }
}

void TaskFilterProxyModel::setFilterTitle(const QString &title)
{
    if (m_filterTitle != title) {
        m_filterTitle = title;
        invalidateFilter();
    }
}

void TaskFilterProxyModel::setFilterStatus(const QString &status)
{
    if (m_filterStatus != status) {
        m_filterStatus = status;
        invalidateFilter();
    }
}

void TaskFilterProxyModel::setFilterPriority(const QString& prio)
{
    if (m_filterPriority != prio) {
        m_filterPriority = prio;
        invalidateFilter();
    }
}

void TaskFilterProxyModel::setFilterDeadlineType(int type)
{
    if (m_filterDeadlineType != type) {
        m_filterDeadlineType = type;
        invalidateFilter();
    }
}

void TaskFilterProxyModel::setFilterIsProjectTask(int isProjectTask) {
    if (m_filterIsProjectTask != isProjectTask) {
        m_filterIsProjectTask = isProjectTask;
        invalidateFilter();
    }
}

bool TaskFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if (!index.isValid())
        return false;

    Task task = index.data(TaskModel::FullTaskRole).value<Task>();

    // Apply general filters first
    if (!m_filterProjectType.isEmpty() && task.projectType() != m_filterProjectType) {
        return false;
    }
    if (!m_filterTitle.trimmed().isEmpty() && !task.title().contains(m_filterTitle.trimmed(), Qt::CaseInsensitive)) {
        return false;
    }
    if (m_filterDate.isValid()) {
        bool dateMatches = false;
        if (task.isProjectTask()) {
            dateMatches = (task.startDateTime().date() <= m_filterDate && m_filterDate <= task.endDateTime().date());
        } else {
            dateMatches = (task.dueDateTime().date() == m_filterDate);
        }
        if (!dateMatches) return false;
    }
    if (!m_filterPriority.isEmpty() && task.priority() != m_filterPriority) {
        return false;
    }
    if (m_filterIsProjectTask == 0 && task.isProjectTask()) return false;
    if (m_filterIsProjectTask == 1 && !task.isProjectTask()) return false;


    // Now, handle status filtering, which is the most complex part.
    if (m_filterDeadlineType == 3) { // "Только выполненные"
        return task.status() == "Выполнено";
    }

    // Apply the regular status filter. If "Выполнено" is selected, show only them.
    if (!m_filterStatus.isEmpty()) {
        return task.status() == m_filterStatus;
    }

    if (m_filterDeadlineType == 2) { // "Только просроченные"
        return task.status() == "Просрочено";
    }

    // For all other modes, hide completed and overdue tasks.
    if (task.status() == "Выполнено" || task.status() == "Просрочено") {
        return false;
    }

    // Apply "Upcoming" filter logic.
    if (m_filterDeadlineType == 1 && task.status() == "Отложено") {
        return false;
    }

    return true;
}

bool TaskFilterProxyModel::filterAcceptsRow_IgnoreDeadline(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if (!index.isValid())
        return false;

    Task task = index.data(TaskModel::FullTaskRole).value<Task>();

    // This function is for the overlay, which only shows timed tasks for a specific day.
    if (!task.isProjectTask()) {
        return false;
    }

    if (m_filterDate.isValid()) {
        if (task.startDateTime().date() > m_filterDate || task.endDateTime().date() < m_filterDate) {
            return false;
        }
    }


    // The overlay should not be affected by other filters like project, title, etc.
    // It shows ALL timed tasks for the selected day.

    return true;
}
