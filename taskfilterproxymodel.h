#ifndef TASKFILTERPROXYMODEL_H
#define TASKFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QDate>
#include "task.h"

class TaskFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit TaskFilterProxyModel(QObject *parent = nullptr);

    void setFilterDate(const QDate &date);
    void setFilterProjectType(const QString &projectType);
    void setFilterTitle(const QString &title);
    void setFilterStatus(const QString &status);
    void setFilterPriority(const QString &prio);
    void setFilterDeadlineType(int type); // 0: все, 1: предстоящие, 2: просроченные
    void setFilterIsProjectTask(int isProjectTask); // -1: все, 0: только без времени, 1: только по времени

    QDate filterDate() const { return m_filterDate; }
    bool filterAcceptsRow_IgnoreDeadline(int source_row, const QModelIndex &source_parent) const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QDate m_filterDate;
    QString m_filterProjectType;
    QString m_filterTitle;
    QString m_filterStatus;
    QString m_filterPriority;
    int m_filterDeadlineType; // 0: все, 1: предстоящие, 2: просроченные
    int m_filterIsProjectTask = -1;
};

#endif // TASKFILTERPROXYMODEL_H
