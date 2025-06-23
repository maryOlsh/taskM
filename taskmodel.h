#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include "task.h"
#include <QUuid>

class CustomDataManager;

class TaskModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        ProjectRole,
        DueDateRole,
        StatusRole,
        DescriptionRole,
        IsProjectRole = Qt::UserRole + 7,
        StartDateRole,
        EndDateRole,
        CreationDateRole,
        FullTaskRole
    };
    enum Columns {
        TitleColumn = 0,
        DateColumn,
        ProjectColumn,
        PriorityColumn,
        StatusColumn,
        ColumnCount
    };

    explicit TaskModel(QObject *parent = nullptr, CustomDataManager *dataManager = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addTask(const Task& task);
    void removeTask(int index);
    void updateTask(int index, const Task& task);
    Task getTask(int index) const;
    const QVector<Task>& tasks() const;
    void clear();
    int findTask(const QUuid& uid) const;

    QVector<int> findTasksUsingProject(const QString& projectName) const;
    QVector<int> findTasksUsingStatus(const QString& status) const;
    QVector<int> findTasksUsingPriority(const QString& priority) const;

    void replaceProjectInTasks(const QVector<int>& taskIndices, const QString& newProject);
    void replaceStatusInTasks(const QVector<int>& taskIndices, const QString& newStatus);
    void replacePriorityInTasks(const QVector<int>& taskIndices, const QString& newPriority);

    bool saveTasks() const;
    bool loadTasks();

private:
    QVector<Task> m_tasks;
    CustomDataManager *m_dataManager;
};

#endif // TASKMODEL_H
