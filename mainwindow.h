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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onAddProject();
    void onAddStatus();
    void onAddPriority();
    void updateCombos();
    void onRemoveProject();
    void onRemoveStatus();
    void onRemovePriority();
    void checkForOverdueTasks();

private:
    void setupUI();
    void setupConnections();
    void addTask();
    void addTaskForTime(int hour);
    void editTask();
    void deleteTask();
    void updateFilterTitle();
    void updateTimeSlotsTable();
    void showTaskDetails(const QModelIndex &index);
    void handleTimeSlotDoubleClick(int row, int column);
    void refreshAllViews();
    bool showTaskDialog(const Task &task, bool isEditMode = false);
    void updateOverdueTasks();
    void exportToCSV();
    void performExport(bool exportAll);
    void saveTasks();
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
