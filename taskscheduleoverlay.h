/**
 * @file taskscheduleoverlay.h
 * @brief Заголовочный файл для TaskScheduleOverlay — оверлея расписания задач.
 */
#ifndef TASKSCHEDULEOVERLAY_H
#define TASKSCHEDULEOVERLAY_H

#include <QWidget>
#include <QVector>
#include <QList> // Added
#include "task.h"
#include "taskslot.h"

class QTableWidget;
class TaskModel;
class TaskFilterProxyModel;
class CustomDataManager;
class QTimer;

/**
 * @struct OverlayTaskRect
 * @brief Прямоугольник для отображения задачи на оверлее.
 */
struct OverlayTaskRect {
    QRect rect;
    Task task;
};

/**
 * @class TaskScheduleOverlay
 * @brief Оверлей для визуализации задач по времени поверх QTableWidget.
 */
class TaskScheduleOverlay : public QWidget {
    Q_OBJECT
public:
    TaskScheduleOverlay(QTableWidget *table, TaskModel *model, TaskFilterProxyModel *proxyModel, CustomDataManager *dataManager, QWidget *parent = nullptr);
    /**
     * @brief Обновляет оверлей задач.
     */
    void updateOverlay();
    /**
     * @brief Устанавливает выбранную дату для отображения.
     * @param date Дата.
     */
    void setSelectedDate(const QDate& date); // новый метод
    /**
     * @brief Игнорировать следующий клик мыши (для предотвращения ложных срабатываний).
     */
    void ignoreNextClick();

signals:
    /**
     * @brief Сигнал для запроса редактирования задачи.
     * @param task Задача.
     * @param proxyRow Строка в прокси-модели.
     */
    void editTaskRequested(const Task &task, int proxyRow);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QTableWidget *m_table;
    TaskModel *m_model;
    TaskFilterProxyModel *m_proxyModel;
    CustomDataManager *m_dataManager;
    QVector<OverlayTaskRect> m_taskRects;
    QList<Task> m_overflowTasks; // Added for overflow tasks
    QPoint m_lastMousePos;
    int m_hoveredTaskIndex = -1;
    QDate m_selectedDate; // новое поле
    bool m_ignoreNextClick = false;
    QTimer* m_timelineTimer;
    void recalculateRects();
    void showTaskTooltip(const QPoint &pos, const Task &task);
    void showOverflowPopup(const QPoint &pos, const QList<Task> &tasks);
};

#endif // TASKSCHEDULEOVERLAY_H
