/**
 * @file mainwindow.h
 * @brief Главное окно приложения TaskM.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "taskmodel.h"
#include "taskfilterproxymodel.h"

class CustomDataManager;
class QListView;
class QDateEdit;
class QComboBox;
class QLabel;
class QLineEdit;
class QTableWidget;
class QTableWidgetItem;
class QTableView;
class QPushButton;
class TaskScheduleOverlay;
class QTimer;

/**
 * @class MainWindow
 * @brief Главное окно приложения для управления задачами.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор MainWindow.
     * @param parent Родительский виджет.
     */
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    /**
     * @brief Фильтр событий для обработки пользовательских действий.
     * @param obj Объект.
     * @param event Событие.
     * @return true если обработано.
     */
    bool eventFilter(QObject *obj, QEvent *event) override;
    /**
     * @brief Обработка изменения размера окна.
     * @param event Событие изменения размера.
     */
    void resizeEvent(QResizeEvent *event) override;
    /**
     * @brief Обработка закрытия окна.
     * @param event Событие закрытия.
     */
    void closeEvent(QCloseEvent *event) override;

private slots:
    /**
     * @brief Слот для добавления проекта.
     */
    void onAddProject();
    /**
     * @brief Слот для добавления статуса.
     */
    void onAddStatus();
    /**
     * @brief Слот для добавления приоритета.
     */
    void onAddPriority();
    /**
     * @brief Обновление выпадающих списков фильтров.
     */
    void updateCombos();
    /**
     * @brief Слот для удаления проекта.
     */
    void onRemoveProject();
    /**
     * @brief Слот для удаления статуса.
     */
    void onRemoveStatus();
    /**
     * @brief Слот для удаления приоритета.
     */
    void onRemovePriority();
    /**
     * @brief Проверка просроченных задач.
     */
    void checkForOverdueTasks();

private:
    /**
     * @brief Инициализация пользовательского интерфейса.
     */
    void setupUI();
    /**
     * @brief Установка соединений сигналов и слотов.
     */
    void setupConnections();
    /**
     * @brief Добавить задачу.
     */
    void addTask();
    /**
     * @brief Добавить задачу для определённого времени.
     * @param hour Час.
     */
    void addTaskForTime(int hour);
    /**
     * @brief Редактировать выбранную задачу.
     */
    void editTask();
    /**
     * @brief Удалить выбранную задачу.
     */
    void deleteTask();
    /**
     * @brief Обновить фильтр по названию.
     */
    void updateFilterTitle();
    /**
     * @brief Обновить таблицу временных слотов.
     */
    void updateTimeSlotsTable();
    /**
     * @brief Показать детали задачи.
     * @param index Индекс задачи.
     */
    void showTaskDetails(const QModelIndex &index);
    /**
     * @brief Обработать двойной клик по временному слоту.
     * @param row Строка.
     * @param column Колонка.
     */
    void handleTimeSlotDoubleClick(int row, int column);
    /**
     * @brief Обновить все представления.
     */
    void refreshAllViews();
    /**
     * @brief Показать диалог задачи.
     * @param task Задача.
     * @param isEditMode Режим редактирования.
     * @return true если задача изменена/создана.
     */
    bool showTaskDialog(const Task &task, bool isEditMode = false);
    /**
     * @brief Обновить просроченные задачи.
     */
    void updateOverdueTasks();
    /**
     * @brief Экспортировать задачи в CSV.
     */
    void exportToCSV();
    /**
     * @brief Выполнить экспорт задач.
     * @param exportAll Экспортировать все или только отфильтрованные.
     */
    void performExport(bool exportAll);
    /**
     * @brief Сохранить задачи.
     */
    void saveTasks();
    /**
     * @brief Загрузить задачи.
     */
    void loadTasks();

    CustomDataManager *m_dataManager;
    TaskModel *taskModel;
    TaskFilterProxyModel *proxyModel;
    QTableView *allTasksView;
    QTableWidget *timeSlotsTable;
    QDateEdit *dateFilterEdit;
    QComboBox *taskTypeCombo;
    QLabel *filterTitleLabel;
    QLineEdit *titleFilterEdit;
    QComboBox *statusFilterCombo;
    QComboBox *priorityFilterCombo;
    QComboBox *deadlineFilterCombo;
    QComboBox *projectFilterCombo;
    QComboBox *isProjectTaskFilterCombo;
    QPushButton *todayButton;
    TaskScheduleOverlay *overlay; // overlay для задач по времени
    bool blockEditOnAdd = false;
    QTimer *m_overdueTaskTimer;
};

#endif // MAINWINDOW_H
