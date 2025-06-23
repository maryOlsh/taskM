/**
 * @file taskmodel.cpp
 * @brief Реализация модели задач.
 */
#include "taskmodel.h"
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <QMessageBox>
#include "customdatamanager.h"
#include <QUuid> // Added for QUuid
#include <QApplication>
#include <QStyle>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>

TaskModel::TaskModel(QObject *parent, CustomDataManager *dataManager)
    : QAbstractListModel(parent), m_dataManager(dataManager)
{}

int TaskModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_tasks.size();
}

int TaskModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant TaskModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_tasks.size())
        return QVariant();

    const Task &task = m_tasks[index.row()];

    if (role == Qt::DecorationRole && index.column() == TitleColumn) {
        if (task.wasModified()) {
            return QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
        }
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case TitleColumn:
            return task.title();
        case DateColumn:
            return Task::formatDate(task);
        case ProjectColumn:
            return task.projectType();
        case PriorityColumn:
            return task.priority();
        case StatusColumn:
            return task.status();
        default:
            return QVariant();
        }
    }
    if (role == Qt::DecorationRole && index.column() == PriorityColumn) {
        QString prio = task.priority();
        if (m_dataManager) {
            QColor color = m_dataManager->getPriorityColor(prio);
            QPixmap pix(16, 16);
            pix.fill(Qt::transparent);
            QPainter p(&pix);
            p.setRenderHint(QPainter::Antialiasing);
            p.setBrush(color);
            p.setPen(Qt::NoPen);
            p.drawEllipse(2, 2, 12, 12);
            p.end();
            return QIcon(pix);
        }
    }
    // Для фильтрации и других ролей
    if (role == FullTaskRole) {
        return QVariant::fromValue(task);
    }
    // Для сортировки и других ролей можно добавить дополнительные case
    return QVariant();
}

QVariant TaskModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case TitleColumn: return "Название";
        case DateColumn: return "Дата";
        case ProjectColumn: return "Проект";
        case PriorityColumn: return "Приоритет";
        case StatusColumn: return "Статус";
        default: return QVariant();
        }
    }
    return QAbstractListModel::headerData(section, orientation, role);
}

// taskmodel.cpp (unchanged, for reference)
bool TaskModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= m_tasks.size())
        return false;

    Task &task = m_tasks[index.row()];

    switch (role) {
    case TitleRole:
        task.setTitle(value.toString());
        break;
    case ProjectRole:
        task.setProjectType(value.toString());
        break;
    case DueDateRole:
        task.setDueDateTime(value.toDateTime()); // Now valid
        break;
    case StatusRole:
        task.setStatus(value.toString());
        break;
    case DescriptionRole:
        task.setDescription(value.toString());
        break;
    case FullTaskRole:
        if (value.canConvert<Task>()) {
            task = value.value<Task>();
        }
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, {role});
    return true;
}

Qt::ItemFlags TaskModel::flags(const QModelIndex &index) const {
    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

QHash<int, QByteArray> TaskModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[TitleRole] = "title";
    roles[ProjectRole] = "project";
    roles[DueDateRole] = "dueDate";
    roles[StatusRole] = "status";
    roles[DescriptionRole] = "description";
    roles[IsProjectRole] = "isProject";
    roles[StartDateRole] = "startDate";
    roles[EndDateRole] = "endDate";
    roles[CreationDateRole] = "creationDate";
    roles[FullTaskRole] = "fullTask";
    return roles;
}

void TaskModel::addTask(const Task& task) {
    // Удалено ограничение на лимит пересечений по времени
    qDebug() << "Adding task to model:"
             << "title:" << task.title()
             << "isProjectTask:" << task.isProjectTask()
             << "projectType:" << task.projectType()
             << "start:" << task.startDateTime()
             << "end:" << task.endDateTime();

    beginInsertRows(QModelIndex(), m_tasks.size(), m_tasks.size());
    m_tasks.append(task);
    endInsertRows();

    qDebug() << "Total tasks in model:" << m_tasks.size();
}

void TaskModel::removeTask(int index) {
    if (index < 0 || index >= m_tasks.size()) return;

    qDebug() << "Removing task at index" << index << ":" << m_tasks[index].title();

    beginRemoveRows(QModelIndex(), index, index);
    m_tasks.removeAt(index);
    endRemoveRows();

    qDebug() << "Total tasks in model:" << m_tasks.size();
}

void TaskModel::updateTask(int index, const Task& task) {
    if (index < 0 || index >= m_tasks.size()) return;

    qDebug() << "Updating task at index" << index
             << "old title:" << m_tasks[index].title()
             << "new title:" << task.title()
             << "isProjectTask:" << task.isProjectTask()
             << "projectType:" << task.projectType()
             << "start:" << task.startDateTime()
             << "end:" << task.endDateTime();

    m_tasks[index] = task;
    emit dataChanged(createIndex(index, 0), createIndex(index, 0));
}

Task TaskModel::getTask(int index) const {
    if (index >= 0 && index < m_tasks.size())
        return m_tasks[index];
    return Task();
}

const QVector<Task>& TaskModel::tasks() const {
    return m_tasks;
}

void TaskModel::clear() {
    if (m_tasks.isEmpty())
        return;

    qDebug() << "Clearing all tasks from model. Total tasks:" << m_tasks.size();

    beginRemoveRows(QModelIndex(), 0, m_tasks.size() - 1);
    m_tasks.clear();
    endRemoveRows();
}

int TaskModel::findTask(const QUuid &uid) const
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].uid() == uid) {
            return i;
        }
    }
    return -1;
}

QVector<int> TaskModel::findTasksUsingProject(const QString &projectName) const
{
    QVector<int> indices;
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].projectType() == projectName) {
            indices.append(i);
        }
    }
    return indices;
}

QVector<int> TaskModel::findTasksUsingStatus(const QString &status) const
{
    QVector<int> indices;
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].status() == status) {
            indices.append(i);
        }
    }
    return indices;
}

QVector<int> TaskModel::findTasksUsingPriority(const QString &priority) const
{
    QVector<int> indices;
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].priority() == priority) {
            indices.append(i);
        }
    }
    return indices;
}

void TaskModel::replaceProjectInTasks(const QVector<int> &taskIndices, const QString &newProject)
{
    for (int index : taskIndices) {
        if (index >= 0 && index < m_tasks.size()) {
            m_tasks[index].setProjectType(newProject);
            m_tasks[index].setWasModified(true);
            emit dataChanged(createIndex(index, 0), createIndex(index, columnCount() - 1));
        }
    }
}

void TaskModel::replaceStatusInTasks(const QVector<int> &taskIndices, const QString &newStatus)
{
    for (int index : taskIndices) {
        if (index >= 0 && index < m_tasks.size()) {
            m_tasks[index].setStatus(newStatus);
            m_tasks[index].setWasModified(true);
            emit dataChanged(createIndex(index, 0), createIndex(index, columnCount() - 1));
        }
    }
}

void TaskModel::replacePriorityInTasks(const QVector<int> &taskIndices, const QString &newPriority)
{
    for (int index : taskIndices) {
        if (index >= 0 && index < m_tasks.size()) {
            m_tasks[index].setPriority(newPriority);
            m_tasks[index].setWasModified(true);
            emit dataChanged(createIndex(index, 0), createIndex(index, columnCount() - 1));
        }
    }
}

bool TaskModel::saveTasks() const
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString filePath = path + "/tasks.json";

    QJsonArray tasksArray;
    for (const Task &task : m_tasks) {
        QJsonObject taskObj;
        taskObj["uid"] = task.uid().toString();
        taskObj["title"] = task.title();
        taskObj["description"] = task.description();
        taskObj["projectType"] = task.projectType();
        taskObj["status"] = task.status();
        taskObj["priority"] = task.priority();
        taskObj["isProjectTask"] = task.isProjectTask();
        taskObj["wasModified"] = task.wasModified();
        taskObj["creationDateTime"] = task.creationDateTime().toString(Qt::ISODate);
        taskObj["startDateTime"] = task.startDateTime().toString(Qt::ISODate);
        taskObj["endDateTime"] = task.endDateTime().toString(Qt::ISODate);
        tasksArray.append(taskObj);
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    file.write(QJsonDocument(tasksArray).toJson());
    file.close();
    return true;
}

bool TaskModel::loadTasks()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = path + "/tasks.json";

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open tasks file.");
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));

    if (!doc.isArray()) {
        qWarning("Tasks file is not a valid JSON array.");
        return false;
    }

    beginResetModel();
    m_tasks.clear();

    QJsonArray tasksArray = doc.array();
    for (const QJsonValue &value : tasksArray) {
        QJsonObject obj = value.toObject();
        Task task;
        task.setUid(QUuid(obj["uid"].toString()));
        task.setTitle(obj["title"].toString());
        task.setDescription(obj["description"].toString());
        task.setProjectType(obj["projectType"].toString());
        task.setStatus(obj["status"].toString());
        task.setPriority(obj["priority"].toString());
        task.setIsProjectTask(obj["isProjectTask"].toBool());
        task.setWasModified(obj["wasModified"].toBool());
        task.setCreationDateTime(QDateTime::fromString(obj["creationDateTime"].toString(), Qt::ISODate));
        task.setStartDateTime(QDateTime::fromString(obj["startDateTime"].toString(), Qt::ISODate));
        task.setEndDateTime(QDateTime::fromString(obj["endDateTime"].toString(), Qt::ISODate));
        m_tasks.append(task);
    }

    endResetModel();
    return true;
}
