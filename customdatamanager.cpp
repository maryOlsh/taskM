#include "customdatamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

// Helper functions for JSON conversion
QJsonObject colorToJson(const QColor& color) {
    QJsonObject obj;
    obj["r"] = color.red();
    obj["g"] = color.green();
    obj["b"] = color.blue();
    return obj;
}

QColor colorFromJson(const QJsonObject& obj) {
    return QColor(obj["r"].toInt(), obj["g"].toInt(), obj["b"].toInt());
}


CustomDataManager::CustomDataManager(QObject *parent) : QObject(parent)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_filePath = path + "/custom_data.json";
    qDebug() << "Custom data file path:" << m_filePath;

    initializeDefaultData();
    loadData();
}

void CustomDataManager::initializeDefaultData()
{
    // Default Projects
    m_projects["Работа"] = QColor(65, 135, 250);
    m_projects["Учёба"] = QColor(255, 170, 0);
    m_projects["Дом"] = QColor(60, 200, 120);
    m_projects["Обычная задача"] = QColor(100, 100, 100);

    // Default Statuses
    m_statuses = {"Не начато", "В процессе", "Выполнено", "Отложено", "Просрочено"};

    // Default Priorities
    m_priorities["Низкий"] = Qt::green;
    m_priorities["Средний"] = Qt::yellow;
    m_priorities["Высокий"] = Qt::red;

    // System values
    m_systemProjects = {"Обычная задача"};
    m_systemStatuses = {"Не начато", "В процессе", "Выполнено", "Отложено", "Просрочено"};
    m_systemPriorities = {"Средний"};
}


void CustomDataManager::loadData()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open custom data file for reading, using defaults.";
        saveData(); // Save defaults if file doesn't exist
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject rootObj = doc.object();

    if (rootObj.contains("projects")) {
        QJsonArray projectsArray = rootObj["projects"].toArray();
        for (const QJsonValue& val : projectsArray) {
            QJsonObject obj = val.toObject();
            m_projects[obj["name"].toString()] = colorFromJson(obj["color"].toObject());
        }
    }
    if (rootObj.contains("statuses")) {
        QJsonArray statusesArray = rootObj["statuses"].toArray();
        for (const QJsonValue& val : statusesArray) {
            if (!m_statuses.contains(val.toString()))
                m_statuses.append(val.toString());
        }
    }
    if (rootObj.contains("priorities")) {
        QJsonArray prioritiesArray = rootObj["priorities"].toArray();
        for (const QJsonValue& val : prioritiesArray) {
            QJsonObject obj = val.toObject();
            m_priorities[obj["name"].toString()] = colorFromJson(obj["color"].toObject());
        }
    }
    emit dataChanged();
}

void CustomDataManager::saveData()
{
    QJsonObject rootObj;

    QJsonArray projectsArray;
    for(auto it = m_projects.constBegin(); it != m_projects.constEnd(); ++it) {
        QJsonObject obj;
        obj["name"] = it.key();
        obj["color"] = colorToJson(it.value());
        projectsArray.append(obj);
    }
    rootObj["projects"] = projectsArray;

    QJsonArray statusesArray = QJsonArray::fromStringList(m_statuses);
    rootObj["statuses"] = statusesArray;

    QJsonArray prioritiesArray;
    for(auto it = m_priorities.constBegin(); it != m_priorities.constEnd(); ++it) {
        QJsonObject obj;
        obj["name"] = it.key();
        obj["color"] = colorToJson(it.value());
        prioritiesArray.append(obj);
    }
    rootObj["priorities"] = prioritiesArray;

    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Could not open custom data file for writing.";
        return;
    }
    file.write(QJsonDocument(rootObj).toJson());
    file.close();
}


const QMap<QString, QColor>& CustomDataManager::getProjects() const { return m_projects; }
const QStringList& CustomDataManager::getStatuses() const { return m_statuses; }
const QMap<QString, QColor>& CustomDataManager::getPriorities() const { return m_priorities; }

QColor CustomDataManager::getProjectColor(const QString& name) const { return m_projects.value(name, QColor(100, 100, 100)); }
QColor CustomDataManager::getPriorityColor(const QString& name) const { return m_priorities.value(name, Qt::gray); }

bool CustomDataManager::addProject(const QString& name, const QColor& color) {
    if (name.isEmpty() || m_projects.contains(name)) return false;
    m_projects[name] = color;
    saveData();
    emit dataChanged();
    return true;
}

bool CustomDataManager::addStatus(const QString& name) {
    if (name.isEmpty() || m_statuses.contains(name)) return false;
    m_statuses.append(name);
    saveData();
    emit dataChanged();
    return true;
}

bool CustomDataManager::addPriority(const QString& name, const QColor& color) {
    if (name.isEmpty() || m_priorities.contains(name)) return false;
    m_priorities[name] = color;
    saveData();
    emit dataChanged();
    return true;
}

bool CustomDataManager::removeProject(const QString &name)
{
    if (!m_projects.contains(name) || isSystemProject(name)) return false;
    m_projects.remove(name);
    saveData();
    emit dataChanged();
    return true;
}

bool CustomDataManager::removeStatus(const QString &name)
{
    if (!m_statuses.contains(name) || isSystemStatus(name)) return false;
    m_statuses.removeAll(name);
    saveData();
    emit dataChanged();
    return true;
}

bool CustomDataManager::removePriority(const QString &name)
{
    if (!m_priorities.contains(name) || isSystemPriority(name)) return false;
    m_priorities.remove(name);
    saveData();
    emit dataChanged();
    return true;
}

bool CustomDataManager::isSystemProject(const QString &name) const
{
    return m_systemProjects.contains(name);
}

bool CustomDataManager::isSystemStatus(const QString &name) const
{
    return m_systemStatuses.contains(name);
}

bool CustomDataManager::isSystemPriority(const QString &name) const
{
    return m_systemPriorities.contains(name);
}
