/**
 * @file taskfilterproxymodel.h
 * @brief Прокси-модель для фильтрации задач.
 */

#ifndef TASKFILTERPROXYMODEL_H
#define TASKFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QDate>
#include "task.h"

/**
 * @class TaskFilterProxyModel
 * @brief Класс для фильтрации задач по различным критериям.
 */
class TaskFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit TaskFilterProxyModel(QObject *parent = nullptr);

    /**
     * @brief Установить фильтр по дате.
     * @param date Дата.
     */
    void setFilterDate(const QDate &date);
    /**
     * @brief Установить фильтр по типу проекта.
     * @param projectType Тип проекта.
     */
    void setFilterProjectType(const QString &projectType);
    /**
     * @brief Установить фильтр по названию задачи.
     * @param title Название.
     */
    void setFilterTitle(const QString &title);
    /**
     * @brief Установить фильтр по статусу.
     * @param status Статус.
     */
    void setFilterStatus(const QString &status);
    /**
     * @brief Установить фильтр по приоритету.
     * @param prio Приоритет.
     */
    void setFilterPriority(const QString &prio);
    /**
     * @brief Установить фильтр по типу дедлайна.
     * @param type Тип дедлайна.
     */
    void setFilterDeadlineType(int type); // 0: все, 1: предстоящие, 2: просроченные
    /**
     * @brief Установить фильтр по типу задачи (проектная/обычная).
     * @param isProjectTask -1: все, 0: только без времени, 1: только по времени.
     */
    void setFilterIsProjectTask(int isProjectTask);

    /**
     * @brief Получить текущий фильтр по дате.
     * @return Дата.
     */
    QDate filterDate() const { return m_filterDate; }
    /**
     * @brief Проверка задачи для оверлея (игнорирует дедлайн-фильтр).
     * @param source_row Строка исходной модели.
     * @param source_parent Родительский индекс.
     * @return true если задача подходит.
     */
    bool filterAcceptsRow_IgnoreDeadline(int source_row, const QModelIndex &source_parent) const;

protected:
    /**
     * @brief Основная функция фильтрации.
     * @param source_row Строка исходной модели.
     * @param source_parent Родительский индекс.
     * @return true если задача подходит.
     */
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
