#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include "task.h"
#include <QUuid>

class CustomDataManager;

/**
 * @brief Модель задач для отображения и управления задачами.
 *
 * Реализует QAbstractListModel для работы с задачами.
 */
class TaskModel : public QAbstractListModel
{
    Q_OBJECT
public:
    /**
     * @brief Роли для данных задачи.
     */
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
    /**
     * @brief Колонки для отображения задач в таблице.
     */
    enum Columns {
        TitleColumn = 0,
        DateColumn,
        ProjectColumn,
        PriorityColumn,
        StatusColumn,
        ColumnCount
    };

    /**
     * @brief Конструктор TaskModel.
     * @param parent Родительский объект.
     * @param dataManager Менеджер пользовательских данных.
     */
    explicit TaskModel(QObject *parent = nullptr, CustomDataManager *dataManager = nullptr);

    /**
     * @brief Возвращает количество строк (задач).
     * @param parent Родительский индекс.
     * @return Количество задач.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /**
     * @brief Возвращает количество колонок.
     * @param parent Родительский индекс.
     * @return Количество колонок.
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    /**
     * @brief Возвращает данные задачи по индексу и роли.
     * @param index Индекс задачи.
     * @param role Роль данных.
     * @return Значение данных.
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /**
     * @brief Возвращает данные заголовка для секции.
     * @param section Секция.
     * @param orientation Ориентация.
     * @param role Роль.
     * @return Данные заголовка.
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    /**
     * @brief Устанавливает данные задачи по индексу и роли.
     * @param index Индекс задачи.
     * @param value Новое значение.
     * @param role Роль.
     * @return true если успешно.
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    /**
     * @brief Флаги для элемента.
     * @param index Индекс.
     * @return Флаги.
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    /**
     * @brief Возвращает имена ролей.
     * @return Хэш ролей.
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Добавляет задачу.
     * @param task Задача.
     */
    void addTask(const Task& task);
    /**
     * @brief Удаляет задачу по индексу.
     * @param index Индекс задачи.
     */
    void removeTask(int index);
    /**
     * @brief Обновляет задачу по индексу.
     * @param index Индекс задачи.
     * @param task Новая задача.
     */
    void updateTask(int index, const Task& task);
    /**
     * @brief Получает задачу по индексу.
     * @param index Индекс задачи.
     * @return Задача.
     */
    Task getTask(int index) const;
    /**
     * @brief Возвращает все задачи.
     * @return Вектор задач.
     */
    const QVector<Task>& tasks() const;
    /**
     * @brief Очищает все задачи.
     */
    void clear();
    /**
     * @brief Находит задачу по UID.
     * @param uid Уникальный идентификатор.
     * @return Индекс задачи или -1.
     */
    int findTask(const QUuid& uid) const;

    /**
     * @brief Находит задачи, использующие проект.
     * @param projectName Имя проекта.
     * @return Вектор индексов.
     */
    QVector<int> findTasksUsingProject(const QString& projectName) const;
    /**
     * @brief Находит задачи по статусу.
     * @param status Статус.
     * @return Вектор индексов.
     */
    QVector<int> findTasksUsingStatus(const QString& status) const;
    /**
     * @brief Находит задачи по приоритету.
     * @param priority Приоритет.
     * @return Вектор индексов.
     */
    QVector<int> findTasksUsingPriority(const QString& priority) const;

    /**
     * @brief Заменяет проект в задачах.
     * @param taskIndices Индексы задач.
     * @param newProject Новый проект.
     */
    void replaceProjectInTasks(const QVector<int>& taskIndices, const QString& newProject);
    /**
     * @brief Заменяет статус в задачах.
     * @param taskIndices Индексы задач.
     * @param newStatus Новый статус.
     */
    void replaceStatusInTasks(const QVector<int>& taskIndices, const QString& newStatus);
    /**
     * @brief Заменяет приоритет в задачах.
     * @param taskIndices Индексы задач.
     * @param newPriority Новый приоритет.
     */
    void replacePriorityInTasks(const QVector<int>& taskIndices, const QString& newPriority);

    /**
     * @brief Сохраняет задачи в файл.
     * @return true если успешно.
     */
    bool saveTasks() const;
    /**
     * @brief Загружает задачи из файла.
     * @return true если успешно.
     */
    bool loadTasks();

private:
    QVector<Task> m_tasks;
    CustomDataManager *m_dataManager;
};

#endif // TASKMODEL_H
