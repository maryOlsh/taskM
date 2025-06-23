#ifndef CUSTOMDATAMANAGER_H
#define CUSTOMDATAMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QMap>
#include <QStringList>

/**
 * @file customdatamanager.h
 * @brief Заголовочный файл для CustomDataManager — менеджера пользовательских данных.
 */

class CustomDataManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Конструктор класса CustomDataManager.
     * @param parent Родительский объект.
     */
    explicit CustomDataManager(QObject *parent = nullptr);

    /**
     * @brief Загружает пользовательские данные из файла.
     */
    void loadData();
    /**
     * @brief Сохраняет пользовательские данные в файл.
     */
    void saveData();

    /**
     * @brief Возвращает карту проектов.
     * @return QMap с названиями и цветами проектов.
     */
    const QMap<QString, QColor>& getProjects() const;
    /**
     * @brief Возвращает список статусов.
     * @return QStringList статусов.
     */
    const QStringList& getStatuses() const;
    /**
     * @brief Возвращает карту приоритетов.
     * @return QMap с названиями и цветами приоритетов.
     */
    const QMap<QString, QColor>& getPriorities() const;

    /**
     * @brief Получить цвет проекта по имени.
     * @param name Имя проекта.
     * @return Цвет.
     */
    QColor getProjectColor(const QString& name) const;
    /**
     * @brief Получить цвет приоритета по имени.
     * @param name Имя приоритета.
     * @return Цвет.
     */
    QColor getPriorityColor(const QString& name) const;

    /**
     * @brief Добавить проект.
     * @param name Имя проекта.
     * @param color Цвет.
     * @return true если успешно.
     */
    bool addProject(const QString& name, const QColor& color);
    /**
     * @brief Добавить статус.
     * @param name Имя статуса.
     * @return true если успешно.
     */
    bool addStatus(const QString& name);
    /**
     * @brief Добавить приоритет.
     * @param name Имя приоритета.
     * @param color Цвет.
     * @return true если успешно.
     */
    bool addPriority(const QString& name, const QColor& color);

    /**
     * @brief Удалить проект.
     * @param name Имя проекта.
     * @return true если успешно.
     */
    bool removeProject(const QString& name);
    /**
     * @brief Удалить статус.
     * @param name Имя статуса.
     * @return true если успешно.
     */
    bool removeStatus(const QString& name);
    /**
     * @brief Удалить приоритет.
     * @param name Имя приоритета.
     * @return true если успешно.
     */
    bool removePriority(const QString& name);

    /**
     * @brief Проверить, является ли проект системным.
     * @param name Имя проекта.
     * @return true если системный.
     */
    bool isSystemProject(const QString& name) const;
    /**
     * @brief Проверить, является ли статус системным.
     * @param name Имя статуса.
     * @return true если системный.
     */
    bool isSystemStatus(const QString& name) const;
    /**
     * @brief Проверить, является ли приоритет системным.
     * @param name Имя приоритета.
     * @return true если системный.
     */
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
