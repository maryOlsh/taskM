#ifndef CUSTOMDATAMANAGER_H
#define CUSTOMDATAMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QMap>
#include <QStringList>

class CustomDataManager : public QObject
{
    Q_OBJECT
public:
    explicit CustomDataManager(QObject *parent = nullptr);

    void loadData();
    void saveData();

    const QMap<QString, QColor>& getProjects() const;
    const QStringList& getStatuses() const;
    const QMap<QString, QColor>& getPriorities() const;

    QColor getProjectColor(const QString& name) const;
    QColor getPriorityColor(const QString& name) const;

    bool addProject(const QString& name, const QColor& color);
    bool addStatus(const QString& name);
    bool addPriority(const QString& name, const QColor& color);

    bool removeProject(const QString& name);
    bool removeStatus(const QString& name);
    bool removePriority(const QString& name);

    bool isSystemProject(const QString& name) const;
    bool isSystemStatus(const QString& name) const;
    bool isSystemPriority(const QString& name) const;

signals:
    void dataChanged();

private:
    void initializeDefaultData();

    QMap<QString, QColor> m_projects;
    QStringList m_statuses;
    QMap<QString, QColor> m_priorities;
    QStringList m_systemProjects;
    QStringList m_systemStatuses;
    QStringList m_systemPriorities;

    QString m_filePath;
};

#endif // CUSTOMDATAMANAGER_H
