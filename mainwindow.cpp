#include <QApplication>
#include "mainwindow.h"
#include "taskdialog.h"
#include "taskdelegate.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QListView>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QToolBar>
#include <QMessageBox>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QDebug>
#include <QMenu>
#include <QContextMenuEvent>
#include "taskscheduleoverlay.h"
#include <QCheckBox>
#include <QUuid>
#include <QTableView>
#include <QFileDialog>
#include <QTextStream>
#include <QRegularExpression>
#include <QStringConverter>
#include "customdatamanager.h"
#include "namedialog.h"
#include "namecolordialog.h"
#include <QInputDialog>
#include <QCloseEvent>
#include <QTimer>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_dataManager(new CustomDataManager(this)),
    taskModel(new TaskModel(this, m_dataManager)),
    proxyModel(new TaskFilterProxyModel(this)),
    allTasksView(nullptr),
    timeSlotsTable(nullptr),
    dateFilterEdit(nullptr),
    taskTypeCombo(nullptr),
    filterTitleLabel(nullptr),
    titleFilterEdit(nullptr),
    statusFilterCombo(nullptr),
    priorityFilterCombo(nullptr),
    deadlineFilterCombo(nullptr),
    projectFilterCombo(nullptr),
    isProjectTaskFilterCombo(nullptr),
    todayButton(nullptr),
    overlay(nullptr)
{
    proxyModel->setSourceModel(taskModel);
    setupUI();
    setupConnections();
    loadTasks();
    refreshAllViews();

    m_overdueTaskTimer = new QTimer(this);
    connect(m_overdueTaskTimer, &QTimer::timeout, this, &MainWindow::checkForOverdueTasks);
    m_overdueTaskTimer->start(60000); // Check every minute
    checkForOverdueTasks(); // Initial check on startup

    // Scroll to current time
    int currentHour = QTime::currentTime().hour();
    if (timeSlotsTable && timeSlotsTable->rowCount() > currentHour) {
        timeSlotsTable->scrollToItem(timeSlotsTable->item(currentHour, 0), QAbstractItemView::PositionAtTop);
    }
}

MainWindow::~MainWindow()
{
    // Автоматическое удаление дочерних объектов
}

void MainWindow::setupUI() {
    // Создание виджетов
    allTasksView = new QTableView(this);
    allTasksView->setModel(proxyModel); // Используем прокси-модель для фильтрации
    allTasksView->setSelectionMode(QAbstractItemView::SingleSelection);
    allTasksView->setContextMenuPolicy(Qt::CustomContextMenu);
    allTasksView->setSortingEnabled(true);
    allTasksView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    allTasksView->installEventFilter(this);
    allTasksView->setEditTriggers(QAbstractItemView::NoEditTriggers); // Отключить редактирование
    allTasksView->setWordWrap(true);
    allTasksView->verticalHeader()->setVisible(false);
    allTasksView->setColumnWidth(1, 240); // Сделать столбец даты шире для полной даты и времени
    allTasksView->resizeRowsToContents();

    timeSlotsTable = new QTableWidget(24, 1, this);
    timeSlotsTable->setHorizontalHeaderLabels({"Задачи"});

    QStringList verticalLabels;
    for (int i = 0; i < 24; ++i) {
        verticalLabels << QString("%1:00").arg(i, 2, 10, QChar('0'));
    }
    timeSlotsTable->setVerticalHeaderLabels(verticalLabels);

    // Настройки внешнего вида
    timeSlotsTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    timeSlotsTable->verticalHeader()->setDefaultSectionSize(60);
    timeSlotsTable->horizontalHeader()->setStretchLastSection(true);

    // Важные настройки для отключения подсветки
    timeSlotsTable->setSelectionMode(QAbstractItemView::NoSelection); // Отключаем выделение
    timeSlotsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Запрещаем редактирование
    timeSlotsTable->setFocusPolicy(Qt::NoFocus); // Убираем фокус
    timeSlotsTable->setStyleSheet(
        "QTableWidget {"
        "   background-color: #6C6C6C;"
        "   gridline-color: #D0D0D0;"
        "}"
        "QTableWidget::item {"
        "   border: none;"
        "   padding: 2px;"
        "}"
        );

    // Компоновка всех задач
    QGroupBox *tasksGroup = new QGroupBox("Все задачи", this);
    QVBoxLayout *tasksLayout = new QVBoxLayout;
    QLineEdit *quickAddEdit = new QLineEdit(this);
    quickAddEdit->setPlaceholderText("Быстрое добавление: Название");
    connect(quickAddEdit, &QLineEdit::returnPressed, this, [this, quickAddEdit]() {
        QString text = quickAddEdit->text().trimmed();
        if (text.isEmpty()) return;

        QString title = text;
        QString project = "Обычная задача";

        QRegularExpression re("(.+)\\s+#(.+)$");
        QRegularExpressionMatch match = re.match(text);

        if (match.hasMatch()) {
            QString potentialTitle = match.captured(1).trimmed();
            QString potentialProject = match.captured(2).trimmed();
            if (m_dataManager->getProjects().contains(potentialProject)) {
                title = potentialTitle;
                project = potentialProject;
            }
        }

        Task task;
        task.setTitle(title);
        task.setProjectType(project);
        task.setIsProjectTask(false);
        task.setStartDateTime(QDateTime(QDate::currentDate(), QTime(0,0)));
        task.setEndDateTime(QDateTime(QDate::currentDate(), QTime(0,0)));
        task.setStatus("Не начато");
        taskModel->addTask(task);

        quickAddEdit->clear();
        refreshAllViews();
        allTasksView->clearSelection();
        allTasksView->setCurrentIndex(QModelIndex());
        allTasksView->setFocusPolicy(Qt::NoFocus);
        quickAddEdit->setFocus();
    });
    tasksLayout->addWidget(quickAddEdit);
    tasksLayout->addWidget(allTasksView);
    tasksGroup->setLayout(tasksLayout);

    // Компоновка задач по времени
    QGroupBox *timeGroup = new QGroupBox("Задачи по времени", this);
    QVBoxLayout *timeLayout = new QVBoxLayout;
    timeLayout->addWidget(timeSlotsTable);
    timeGroup->setLayout(timeLayout);

    // --- ФИЛЬТРЫ ---
    projectFilterCombo = new QComboBox(this);
    projectFilterCombo->addItem("Все проекты", ""); // Use empty string for "all"

    titleFilterEdit = new QLineEdit(this);
    titleFilterEdit->setPlaceholderText("Фильтр по названию...");

    dateFilterEdit = new QDateEdit(QDate::currentDate(), this);
    dateFilterEdit->setCalendarPopup(true);
    todayButton = new QPushButton("Сегодня", this);
    QPushButton *allDatesButton = new QPushButton("Все даты", this);
    QPushButton *resetFiltersButton = new QPushButton("Сбросить все фильтры", this);

    statusFilterCombo = new QComboBox(this);
    statusFilterCombo->addItem("Любой статус", "");
    statusFilterCombo->addItem("Не начато", "Не начато");
    statusFilterCombo->addItem("В процессе", "В процессе");
    statusFilterCombo->addItem("Выполнено", "Выполнено");
    statusFilterCombo->addItem("Отложено", "Отложено");

    priorityFilterCombo = new QComboBox(this);
    priorityFilterCombo->addItem("Любой приоритет", "");
    priorityFilterCombo->addItem("Низкий", "Низкий");
    priorityFilterCombo->addItem("Средний", "Средний");
    priorityFilterCombo->addItem("Высокий", "Высокий");

    deadlineFilterCombo = new QComboBox(this);
    deadlineFilterCombo->addItem("Все задачи", 0);
    deadlineFilterCombo->addItem("Только предстоящие", 1);
    deadlineFilterCombo->addItem("Только просроченные", 2);

    isProjectTaskFilterCombo = new QComboBox(this);
    isProjectTaskFilterCombo->addItem("Все", -1);
    isProjectTaskFilterCombo->addItem("Только по времени", 1);
    isProjectTaskFilterCombo->addItem("Только без времени", 0);

    QGroupBox *filterGroup = new QGroupBox("Фильтры", this);
    QVBoxLayout *filterLayout = new QVBoxLayout;
    filterLayout->addWidget(new QLabel("Проект:", this));
    filterLayout->addWidget(projectFilterCombo);
    filterLayout->addSpacing(10);
    filterLayout->addWidget(new QLabel("Название:", this));
    filterLayout->addWidget(titleFilterEdit);
    filterLayout->addSpacing(10);
    filterLayout->addWidget(new QLabel("Дата:", this));
    QHBoxLayout *dateLayout = new QHBoxLayout;
    dateLayout->addWidget(dateFilterEdit);
    dateLayout->addWidget(todayButton);
    dateLayout->addWidget(allDatesButton);
    filterLayout->addLayout(dateLayout);
    filterLayout->addSpacing(10);
    filterLayout->addWidget(new QLabel("Статус:", this));
    filterLayout->addWidget(statusFilterCombo);
    filterLayout->addSpacing(10);
    filterLayout->addWidget(new QLabel("Приоритет:", this));
    filterLayout->addWidget(priorityFilterCombo);
    filterLayout->addSpacing(10);
    filterLayout->addWidget(new QLabel("Дедлайн:", this));
    filterLayout->addWidget(deadlineFilterCombo);
    filterLayout->addSpacing(10);
    filterLayout->addWidget(new QLabel("Задача по времени:", this));
    filterLayout->addWidget(isProjectTaskFilterCombo);
    filterLayout->addWidget(resetFiltersButton);
    filterLayout->addStretch();
    filterGroup->setLayout(filterLayout);

    // --- Layout ---
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(filterGroup, 1);
    mainLayout->addWidget(tasksGroup, 3);
    mainLayout->addWidget(timeGroup, 2);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // Панель инструментов
    QToolBar *toolBar = addToolBar("Действия");
    toolBar->addAction("Добавить задачу", this, &MainWindow::addTask);
    toolBar->addAction("Редактировать", this, &MainWindow::editTask);
    toolBar->addAction("Удалить", this, &MainWindow::deleteTask);
    toolBar->addAction("Экспорт в CSV", this, &MainWindow::exportToCSV);

    // Статус бар
    QPushButton *addButton = new QPushButton("Добавить", this);
    QMenu *addMenu = new QMenu(this);
    addMenu->addAction("Проект", this, &MainWindow::onAddProject);
    addMenu->addAction("Статус", this, &MainWindow::onAddStatus);
    addMenu->addAction("Приоритет", this, &MainWindow::onAddPriority);
    addButton->setMenu(addMenu);
    statusBar()->addWidget(addButton);

    QPushButton *removeButton = new QPushButton("Удалить", this);
    QMenu *removeMenu = new QMenu(this);
    removeMenu->addAction("Удалить проект", this, &MainWindow::onRemoveProject);
    removeMenu->addAction("Удалить статус", this, &MainWindow::onRemoveStatus);
    removeMenu->addAction("Удалить приоритет", this, &MainWindow::onRemovePriority);
    removeButton->setMenu(removeMenu);
    statusBar()->addWidget(removeButton);


    setWindowTitle("Task Manager");
    resize(1200, 700);

    // Создаем оверлей для отображения задач
    qDebug() << "Creating overlay for timeSlotsTable...";
    overlay = new TaskScheduleOverlay(timeSlotsTable, taskModel, proxyModel, m_dataManager, timeSlotsTable->viewport());
    overlay->setGeometry(timeSlotsTable->viewport()->rect());
    overlay->setSelectedDate(QDate::currentDate()); // установка выбранной даты
    overlay->raise();
    overlay->show();

    qDebug() << "Initial overlay geometry:" << overlay->geometry()
             << "Table viewport geometry:" << timeSlotsTable->viewport()->rect();

    // Подключаем сигналы для обновления оверлея
    connect(overlay, &TaskScheduleOverlay::editTaskRequested, this, [this](const Task &task, int proxyRow) {
        if (task.startDateTime().isValid() && proxyRow >= 0) {
            qDebug() << "Edit task requested for time:" << task.startDateTime();
            TaskDialog dialog(m_dataManager, this, task);
            if (dialog.exec() == QDialog::Accepted) {
                Task editedTask = dialog.getTask();
                QModelIndex idx = proxyModel->index(proxyRow, 0);
                QModelIndex sourceIdx = proxyModel->mapToSource(idx);
                taskModel->updateTask(sourceIdx.row(), editedTask);
                refreshAllViews();
            }
        }
    });

    // --- Connections ---
    connect(todayButton, &QPushButton::clicked, this, [this]() {
        // Set date in widget
        dateFilterEdit->setDate(QDate::currentDate());
        // Directly set filter in model to ensure it always applies
        proxyModel->setFilterDate(QDate::currentDate());
        refreshAllViews();
    });
    connect(allDatesButton, &QPushButton::clicked, this, [this]() {
        // Clear the filter in the model
        proxyModel->setFilterDate(QDate());
        refreshAllViews();
    });
    connect(projectFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        proxyModel->setFilterProjectType(projectFilterCombo->currentData().toString());
        refreshAllViews();
    });
    connect(titleFilterEdit, &QLineEdit::textChanged, [this]() {
        proxyModel->setFilterTitle(titleFilterEdit->text());
        refreshAllViews();
    });
    connect(dateFilterEdit, &QDateEdit::dateChanged, [this]() {
        proxyModel->setFilterDate(dateFilterEdit->date());
        refreshAllViews();
    });
    connect(statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        proxyModel->setFilterStatus(statusFilterCombo->currentData().toString());
        refreshAllViews();
    });
    connect(priorityFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        proxyModel->setFilterPriority(priorityFilterCombo->currentData().toString());
        refreshAllViews();
    });
    connect(deadlineFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        proxyModel->setFilterDeadlineType(deadlineFilterCombo->currentData().toInt());
        refreshAllViews();
    });
    connect(isProjectTaskFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        proxyModel->setFilterIsProjectTask(isProjectTaskFilterCombo->currentData().toInt());
        refreshAllViews();
    });
    connect(resetFiltersButton, &QPushButton::clicked, this, [this]() {
        // Сбросить все фильтры
        projectFilterCombo->setCurrentIndex(0); // Все проекты
        titleFilterEdit->clear();
        dateFilterEdit->setDate(QDate::currentDate());
        statusFilterCombo->setCurrentIndex(0); // Любой статус
        priorityFilterCombo->setCurrentIndex(0); // Любой приоритет
        deadlineFilterCombo->setCurrentIndex(0); // Все задачи
        isProjectTaskFilterCombo->setCurrentIndex(0); // Все
        proxyModel->setFilterProjectType("");
        proxyModel->setFilterTitle("");
        proxyModel->setFilterStatus("");
        proxyModel->setFilterPriority("");
        proxyModel->setFilterDeadlineType(0);
        proxyModel->setFilterIsProjectTask(-1);
        proxyModel->setFilterDate(QDate()); // Явно сбрасываем фильтр по дате!
        refreshAllViews();
    });

    connect(allTasksView->horizontalHeader(), &QHeaderView::sectionResized, this, [this]() {
        allTasksView->resizeRowsToContents();
    });

    connect(m_dataManager, &CustomDataManager::dataChanged, this, &MainWindow::updateCombos);
    updateCombos();
}

void MainWindow::setupConnections() {
    // Двойной клик по задаче — редактирование или создание новой задачи
    connect(allTasksView, &QTableView::doubleClicked, this, [this](const QModelIndex &index) {
        if (index.isValid()) {
            // Редактирование существующей задачи
            QModelIndex sourceIndex = proxyModel->mapToSource(index);
            Task originalTask = taskModel->getTask(sourceIndex.row());
            TaskDialog dialog(m_dataManager, this, originalTask);
            connect(&dialog, &TaskDialog::taskDeleted, this, [this, sourceIndex]() {
                taskModel->removeTask(sourceIndex.row());
                refreshAllViews();
                saveTasks();
            });
            if (dialog.exec() == QDialog::Accepted) {
                Task updatedTask = dialog.getTask();
                taskModel->updateTask(sourceIndex.row(), updatedTask);
                refreshAllViews();
                saveTasks();
            }
        } else {
            // Двойной клик по пустому месту — создание новой задачи
            TaskDialog dialog(m_dataManager, this);
            if (dialog.exec() == QDialog::Accepted) {
                Task newTask = dialog.getTask();
                taskModel->addTask(newTask);
                refreshAllViews();
                saveTasks();
            }
        }
    });
    // Двойной клик по таблице времени — создание проектной задачи
    timeSlotsTable->setMouseTracking(true);
    timeSlotsTable->viewport()->installEventFilter(this);

    // Контекстное меню для таблицы времени
    connect(timeSlotsTable, &QWidget::customContextMenuRequested, [this](const QPoint &pos) {
        QTableWidgetItem *item = timeSlotsTable->itemAt(pos);
        if (item) {
            int hour = timeSlotsTable->row(item);
            QMenu menu;
            menu.addAction("Добавить задачу", [this, hour]() { addTaskForTime(hour); });
            menu.exec(timeSlotsTable->viewport()->mapToGlobal(pos));
        }
    });
}

void MainWindow::refreshAllViews() {
    qDebug() << "Refreshing all views...";
    updateTimeSlotsTable();
    if (overlay) {
        qDebug() << "Updating overlay...";
        QDate dateForOverlay = proxyModel->filterDate();
        if (!dateForOverlay.isValid()) {
            dateForOverlay = QDate::currentDate();
        }
        overlay->setSelectedDate(dateForOverlay);
        overlay->updateOverlay();
    }
    // Сброс выделения и фокуса после обновления
    allTasksView->clearSelection();
    allTasksView->setCurrentIndex(QModelIndex());
    allTasksView->clearFocus();
    allTasksView->resizeRowsToContents();
}

void MainWindow::addTask() {
    qDebug() << "Adding new task...";
    TaskDialog dialog(m_dataManager, this);
    dialog.setDueDateTime(QDateTime(QDate::currentDate(), QTime(0, 0)));
    if (dialog.exec() == QDialog::Accepted) {
        Task task = dialog.getTask();
        qDebug() << "Task dialog accepted, adding task to model:"
                 << "title:" << task.title()
                 << "isProjectTask:" << task.isProjectTask()
                 << "projectType:" << task.projectType()
                 << "start:" << task.startDateTime()
                 << "end:" << task.endDateTime();
        taskModel->addTask(task);
        refreshAllViews();
        saveTasks();
    } else {
        qDebug() << "Task dialog rejected";
    }
}

void MainWindow::addTaskForTime(int hour) {
    qDebug() << "Adding task for time:" << hour;
    QDateTime startDateTime = QDateTime(
        dateFilterEdit->date(),
        QTime(hour, 0)
        );
    QDateTime endDateTime = startDateTime.addSecs(3600); // По умолчанию 1 час

    TaskDialog dialog(m_dataManager, this, Task("", "Работа", startDateTime, endDateTime, "В работе", ""));
    dialog.setDueDateTime(startDateTime);

    if (dialog.exec() == QDialog::Accepted) {
        Task task = dialog.getTask();
        qDebug() << "Task dialog accepted, adding task to model:"
                 << "title:" << task.title()
                 << "isProjectTask:" << task.isProjectTask()
                 << "projectType:" << task.projectType()
                 << "start:" << task.startDateTime()
                 << "end:" << task.endDateTime();
        taskModel->addTask(task);
        if (overlay) overlay->ignoreNextClick();
        refreshAllViews();
        saveTasks();
    } else {
        qDebug() << "Task dialog rejected";
    }
}

void MainWindow::editTask() {
    QModelIndex index = allTasksView->currentIndex();
    if (!index.isValid()) {
        qDebug() << "No task selected for editing";
        return;
    }

    // Преобразуем индекс прокси-модели в индекс исходной модели
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    int taskIndex = sourceIndex.row();

    Task originalTask = taskModel->getTask(taskIndex);
    qDebug() << "Editing task:"
             << "title:" << originalTask.title()
             << "isProjectTask:" << originalTask.isProjectTask()
             << "projectType:" << originalTask.projectType()
             << "start:" << originalTask.startDateTime()
             << "end:" << originalTask.endDateTime();

    TaskDialog dialog(m_dataManager, this, originalTask);

    if (dialog.exec() == QDialog::Accepted) {
        Task updatedTask = dialog.getTask();
        qDebug() << "Task dialog accepted, updating task in model:"
                 << "title:" << updatedTask.title()
                 << "isProjectTask:" << updatedTask.isProjectTask()
                 << "projectType:" << updatedTask.projectType()
                 << "start:" << updatedTask.startDateTime()
                 << "end:" << updatedTask.endDateTime();
        taskModel->updateTask(taskIndex, updatedTask);
        refreshAllViews();
        saveTasks();
    } else {
        qDebug() << "Task dialog rejected";
    }
}

void MainWindow::deleteTask() {
    QModelIndex index = allTasksView->currentIndex();
    if (!index.isValid()) return;

    // Преобразуем индекс прокси-модели в индекс исходной модели
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    int taskIndex = sourceIndex.row();

    if (QMessageBox::question(this, "Удаление",
                              "Вы уверены, что хотите удалить задачу?") == QMessageBox::Yes) {
        taskModel->removeTask(taskIndex);
        refreshAllViews();
        saveTasks();
    }
}

void MainWindow::onAddProject()
{
    NameColorDialog dialog("Добавить проект", this);
    if (dialog.exec() == QDialog::Accepted) {
        m_dataManager->addProject(dialog.name(), dialog.color());
    }
}

void MainWindow::onAddStatus()
{
    NameDialog dialog("Добавить статус", this);
    if (dialog.exec() == QDialog::Accepted) {
        m_dataManager->addStatus(dialog.name());
    }
}

void MainWindow::onAddPriority()
{
    NameColorDialog dialog("Добавить приоритет", this);
    if (dialog.exec() == QDialog::Accepted) {
        m_dataManager->addPriority(dialog.name(), dialog.color());
    }
}

void MainWindow::onRemoveProject()
{
    QStringList projects;
    for(const auto& p : m_dataManager->getProjects().keys()) {
        if (!m_dataManager->isSystemProject(p)) {
            projects << p;
        }
    }

    if (projects.isEmpty()) {
        QMessageBox::information(this, "Удаление проекта", "Нет проектов для удаления.");
        return;
    }

    bool ok;
    QString projectToDelete = QInputDialog::getItem(this, "Удалить проект",
                                                    "Выберите проект для удаления:", projects, 0, false, &ok);
    if (!ok || projectToDelete.isEmpty()) return;

    QVector<int> affectedTasks = taskModel->findTasksUsingProject(projectToDelete);
    if (!affectedTasks.isEmpty()) {
        QString msg = QString("Проект '%1' используется в %2 задачах. "
                              "После удаления он будет заменен на 'Обычная задача'.\n\nПродолжить?")
                          .arg(projectToDelete).arg(affectedTasks.size());
        if (QMessageBox::question(this, "Подтверждение удаления", msg) != QMessageBox::Yes) {
            return;
        }
    } else {
        if (QMessageBox::question(this, "Подтверждение удаления", QString("Вы уверены, что хотите удалить проект '%1'?").arg(projectToDelete)) != QMessageBox::Yes) {
            return;
        }
    }

    taskModel->replaceProjectInTasks(affectedTasks, "Обычная задача");
    m_dataManager->removeProject(projectToDelete);
    // updateCombos is called automatically via signal from dataManager
    refreshAllViews();
    saveTasks();
}

void MainWindow::onRemoveStatus()
{
    QStringList statuses;
    for(const auto& s : m_dataManager->getStatuses()) {
        if (!m_dataManager->isSystemStatus(s)) {
            statuses << s;
        }
    }

    if (statuses.isEmpty()) {
        QMessageBox::information(this, "Удаление статуса", "Нет статусов для удаления.");
        return;
    }

    bool ok;
    QString statusToDelete = QInputDialog::getItem(this, "Удалить статус",
                                                   "Выберите статус для удаления:", statuses, 0, false, &ok);
    if (!ok || statusToDelete.isEmpty()) return;

    QVector<int> affectedTasks = taskModel->findTasksUsingStatus(statusToDelete);
    if (!affectedTasks.isEmpty()) {
        QString msg = QString("Статус '%1' используется в %2 задачах. "
                              "После удаления он будет заменен на 'Не начато'.\n\nПродолжить?")
                          .arg(statusToDelete).arg(affectedTasks.size());
        if (QMessageBox::question(this, "Подтверждение удаления", msg) != QMessageBox::Yes) {
            return;
        }
    } else {
        if (QMessageBox::question(this, "Подтверждение удаления", QString("Вы уверены, что хотите удалить статус '%1'?").arg(statusToDelete)) != QMessageBox::Yes) {
            return;
        }
    }

    taskModel->replaceStatusInTasks(affectedTasks, "Не начато");
    m_dataManager->removeStatus(statusToDelete);
    refreshAllViews();
    saveTasks();
}

void MainWindow::onRemovePriority()
{
    QStringList priorities;
    for(const auto& p : m_dataManager->getPriorities().keys()) {
        if (!m_dataManager->isSystemPriority(p)) {
            priorities << p;
        }
    }

    if (priorities.isEmpty()) {
        QMessageBox::information(this, "Удаление приоритета", "Нет приоритетов для удаления.");
        return;
    }

    bool ok;
    QString priorityToDelete = QInputDialog::getItem(this, "Удалить приоритет",
                                                     "Выберите приоритет для удаления:", priorities, 0, false, &ok);
    if (!ok || priorityToDelete.isEmpty()) return;

    QVector<int> affectedTasks = taskModel->findTasksUsingPriority(priorityToDelete);
    if (!affectedTasks.isEmpty()) {
        QString msg = QString("Приоритет '%1' используется в %2 задачах. "
                              "После удаления он будет заменен на 'Средний'.\n\nПродолжить?")
                          .arg(priorityToDelete).arg(affectedTasks.size());
        if (QMessageBox::question(this, "Подтверждение удаления", msg) != QMessageBox::Yes) {
            return;
        }
    } else {
        if (QMessageBox::question(this, "Подтверждение удаления", QString("Вы уверены, что хотите удалить приоритет '%1'?").arg(priorityToDelete)) != QMessageBox::Yes) {
            return;
        }
    }

    taskModel->replacePriorityInTasks(affectedTasks, "Средний");
    m_dataManager->removePriority(priorityToDelete);
    refreshAllViews();
    saveTasks();
}


void MainWindow::updateCombos()
{
    // Block signals to prevent filter changes while repopulating
    projectFilterCombo->blockSignals(true);
    statusFilterCombo->blockSignals(true);
    priorityFilterCombo->blockSignals(true);

    QString currentProject = projectFilterCombo->currentText();
    QString currentStatus = statusFilterCombo->currentText();
    QString currentPriority = priorityFilterCombo->currentText();

    // Projects
    projectFilterCombo->clear();
    projectFilterCombo->addItem("Все проекты", "");
    for (const auto& name : m_dataManager->getProjects().keys()) {
        projectFilterCombo->addItem(name, name);
    }

    // Statuses
    statusFilterCombo->clear();
    statusFilterCombo->addItem("Любой статус", "");
    QStringList statuses = m_dataManager->getStatuses();
    statuses.removeAll("Просрочено");
    for (const auto& name : statuses) {
        statusFilterCombo->addItem(name, name);
    }

    // Priorities
    priorityFilterCombo->clear();
    priorityFilterCombo->addItem("Любой приоритет", "");
    for (const auto& name : m_dataManager->getPriorities().keys()) {
        priorityFilterCombo->addItem(name, name);
    }

    projectFilterCombo->setCurrentText(currentProject);
    statusFilterCombo->setCurrentText(currentStatus);
    priorityFilterCombo->setCurrentText(currentPriority);

    projectFilterCombo->blockSignals(false);
    statusFilterCombo->blockSignals(false);
    priorityFilterCombo->blockSignals(false);
}

void MainWindow::updateTimeSlotsTable() {
    qDebug() << "Updating time slots table...";
    // Отключаем обновление виджета для оптимизации
    timeSlotsTable->setUpdatesEnabled(false);

    // Убедимся, что в таблице 24 строки
    if (timeSlotsTable->rowCount() != 24) {
        timeSlotsTable->setRowCount(24);
    }

    // Очищаем таблицу и устанавливаем стандартные цвета
    for (int row = 0; row < 24; ++row) {
        QTableWidgetItem *item = timeSlotsTable->item(row, 0);
        if (!item) {
            item = new QTableWidgetItem();
            timeSlotsTable->setItem(row, 0, item);
        }
        item->setText("");
        item->setBackground(QColor(240, 240, 240));
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    }

    // Включаем обновление виджета
    timeSlotsTable->setUpdatesEnabled(true);

    if (overlay) {
        qDebug() << "Updating overlay after time slots table update..."
                 << "Overlay geometry:" << overlay->geometry()
                 << "Table viewport geometry:" << timeSlotsTable->viewport()->rect();
        overlay->setGeometry(timeSlotsTable->viewport()->rect());
        overlay->setSelectedDate(QDate::currentDate()); // Синхронизация даты
        overlay->updateOverlay();
    }
}

void MainWindow::showTaskDetails(const QModelIndex &index) {
    Task task;
    if (index.model() == proxyModel) {
        QModelIndex sourceIndex = proxyModel->mapToSource(index);
        task = taskModel->getTask(sourceIndex.row());
    } else {
        return;
    }

    QString details = QString(
                          "<b>%1</b><br>"
                          "Тип задачи: %2<br>"
                          "Срок: %3<br>"
                          "Статус: %4<br>"
                          "Описание: %5"
                          ).arg(task.title(),
                               task.projectType(),
                               task.dueDateTime().toString("dd.MM.yyyy HH:mm"),
                               task.status(),
                               task.description());

    QMessageBox::information(this, "Детали задачи", details);
}

bool MainWindow::showTaskDialog(const Task &task, bool isEditMode) {
    // Проверяем, нет ли уже открытого диалога
    for (QWidget *widget : QApplication::topLevelWidgets()) {
        if (qobject_cast<TaskDialog*>(widget)) {
            return false; // Если диалог уже открыт, игнорируем событие
        }
    }

    TaskDialog dialog(m_dataManager, this, task);
    if (dialog.exec() == QDialog::Accepted) {
        Task updatedTask = dialog.getTask();
        if (isEditMode) {
            QModelIndex index = allTasksView->currentIndex();
            if (index.isValid()) {
                QModelIndex sourceIndex = proxyModel->mapToSource(index);
                taskModel->updateTask(sourceIndex.row(), updatedTask);
            }
        } else {
            updatedTask.setUid(QUuid::createUuid());
            taskModel->addTask(updatedTask);
        }
        refreshAllViews();
        return true;
    }
    return false;
}

void MainWindow::exportToCSV() {
    QMessageBox exportDialog(this);
    exportDialog.setWindowTitle("Экспорт задач");
    exportDialog.setText("Выберите, какие задачи экспортировать.");
    QPushButton *exportFilteredButton = exportDialog.addButton("Экспортировать отфильтрованные", QMessageBox::ActionRole);
    QPushButton *exportAllButton = exportDialog.addButton("Экспортировать все", QMessageBox::ActionRole);
    exportDialog.addButton(QMessageBox::Cancel);

    exportDialog.exec();

    if (exportDialog.clickedButton() == exportFilteredButton) {
        performExport(false); // false for not all
    } else if (exportDialog.clickedButton() == exportAllButton) {
        performExport(true); // true for all
    }
}

void MainWindow::performExport(bool exportAll)
{
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт в CSV", "tasks.csv", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << QChar(0xFEFF); // BOM для Excel

    // Заголовки
    out << "UID;Название;Дата;Тип задачи;Статус;Приоритет;Описание\n";

    auto escape = [](const QString &s) {
        QString res = s;
        res.replace("\"", "\"\"");
        return "\"" + res + "\"";
    };

    int rowCount = exportAll ? taskModel->rowCount() : proxyModel->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        Task t;
        if (exportAll) {
            t = taskModel->getTask(row);
        } else {
            QModelIndex idx = proxyModel->index(row, 0);
            QModelIndex srcIdx = proxyModel->mapToSource(idx);
            t = taskModel->getTask(srcIdx.row());
        }

        QStringList fields;
        fields << escape(t.uid().toString());
        fields << escape(t.title());
        // Дата: для задач по времени — дата + время, иначе просто дата
        if (t.isProjectTask()) {
            QString dateStr = t.startDateTime().date().toString("dd.MM.yyyy");
            QString timeStr = t.startDateTime().time().toString("HH:mm") + "–" + t.endDateTime().time().toString("HH:mm");
            fields << escape(dateStr + " " + timeStr);
        } else {
            fields << escape(t.dueDateTime().date().toString("dd.MM.yyyy"));
        }
        fields << escape(t.projectType());
        fields << escape(t.status());
        fields << escape(t.priority());
        fields << escape(t.description());
        out << fields.join(";") << "\n";
    }
    file.close();
}

// void MainWindow::importFromCSV() {
//     QString fileName = QFileDialog::getOpenFileName(this, "Импорт из CSV", "", "CSV Files (*.csv);;All Files (*)");
//     if (fileName.isEmpty()) return;
//     QFile file(fileName);
//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
//     QTextStream in(&file);
//     in.setEncoding(QStringConverter::Utf8);
//     QString headerLine = in.readLine();
//     if (!headerLine.isEmpty() && headerLine[0] == QChar(0xFEFF))
//         headerLine.remove(0, 1);

//     QStringList headers = headerLine.split(QRegularExpression("[;,]"));
//     int uidColumn = headers.indexOf("UID");

//     QVector<Task> tasksToAdd;
//     QVector<QPair<int, Task>> tasksToUpdate;
//     int imported = 0, skipped = 0;
//     while (!in.atEnd()) {
//         QString line = in.readLine();
//         if (line.trimmed().isEmpty()) continue;
//         QStringList fields;
//         bool inQuotes = false;
//         QString field;
//         for (int i = 0; i < line.size(); ++i) {
//             QChar c = line[i];
//             if (c == '"') {
//                 if (inQuotes && i+1 < line.size() && line[i+1] == '"') {
//                     field += '"'; ++i;
//                 } else {
//                     inQuotes = !inQuotes;
//                 }
//             } else if ((c == ';' || c == ',') && !inQuotes) {
//                 fields << field; field.clear();
//             } else {
//                 field += c;
//             }
//         }
//         fields << field;
//         while (fields.size() < 6) fields << "";

//         auto getField = [&](const QString& name) {
//             int idx = headers.indexOf(name);
//             return (idx != -1 && idx < fields.size()) ? fields[idx].trimmed() : QString();
//         };

//         QString uidStr = (uidColumn != -1 && uidColumn < fields.size()) ? fields[uidColumn].trimmed() : QString();
//         QString title = getField("Название");
//         QString dateStr = getField("Дата");
//         QString projectTypeStr = getField("Тип задачи");
//         QString status = getField("Статус");
//         QString priority = getField("Приоритет");
//         QString description = getField("Описание");

//         QDateTime startDT, endDT;
//         bool isProjectTask = false;
//         QRegularExpression reTimeRange("(\\d{2}\\.\\d{2}\\.\\d{4})? ?(\\d{2}:\\d{2})[–-](\\d{2}:\\d{2})");
//         QRegularExpressionMatch m = reTimeRange.match(dateStr);
//         if (m.hasMatch()) {
//             QDate date = QDate::fromString(m.captured(1).isEmpty() ? QDate::currentDate().toString("dd.MM.yyyy") : m.captured(1), "dd.MM.yyyy");
//             QTime t1 = QTime::fromString(m.captured(2), "HH:mm");
//             QTime t2 = QTime::fromString(m.captured(3), "HH:mm");
//             startDT = QDateTime(date, t1);
//             endDT = QDateTime(date, t2);
//             isProjectTask = true;
//         } else {
//             QDate date = QDate::fromString(dateStr, "dd.MM.yyyy");
//             if (!date.isValid()) date = QDate::currentDate();
//             startDT = QDateTime(date, QTime(0,0));
//             endDT = QDateTime(date, QTime(0,0));
//         }
//         if (title.isEmpty() || !startDT.isValid() || !endDT.isValid()) { skipped++; continue; }
//         Task task;
//         if (!uidStr.isEmpty()) {
//             task.setUid(QUuid(uidStr));
//         }
//         task.setTitle(title);
//         if (!projectTypeStr.isEmpty())
//             task.setProjectType(projectTypeStr);
//         if (!status.isEmpty())
//             task.setStatus(status);
//         if (!priority.isEmpty())
//             task.setPriority(priority);
//         task.setDescription(description);
//         task.setStartDateTime(startDT);
//         task.setEndDateTime(endDT);
//         if (isProjectTask && startDT.isValid() && endDT.isValid() && startDT != endDT) {
//             task.setIsProjectTask(true);
//         } else {
//             task.setIsProjectTask(false);
//         }

//         int existingIndex = -1;
//         if (!uidStr.isEmpty()) {
//             existingIndex = taskModel->findTask(QUuid(uidStr));
//         }

//         if (existingIndex != -1) {
//             tasksToUpdate.append({existingIndex, task});
//         } else {
//             tasksToAdd.append(task);
//         }
//         imported++;
//     }
//     file.close();

//     // Блокируем сигналы модели на время массового добавления
//     taskModel->blockSignals(true);
//     for (const Task& t : tasksToAdd) {
//         taskModel->addTask(t);
//     }
//     for (const auto& pair : tasksToUpdate) {
//         taskModel->updateTask(pair.first, pair.second);
//     }
//     taskModel->blockSignals(false);

//     // Сбросить все фильтры после импорта
//     projectFilterCombo->setCurrentIndex(0); // Все проекты
//     titleFilterEdit->clear();
//     dateFilterEdit->setDate(QDate::currentDate());
//     statusFilterCombo->setCurrentIndex(0);
//     priorityFilterCombo->setCurrentIndex(0);
//     deadlineFilterCombo->setCurrentIndex(0);
//     isProjectTaskFilterCombo->setCurrentIndex(0);
//     proxyModel->setFilterProjectType("");
//     proxyModel->setFilterTitle("");
//     proxyModel->setFilterStatus("");
//     proxyModel->setFilterPriority("");
//     proxyModel->setFilterDeadlineType(0);
//     proxyModel->setFilterIsProjectTask(-1);
//     proxyModel->setFilterDate(QDate());

//     refreshAllViews();
//     QMessageBox::information(this, "Импорт завершён", QString("Импортировано задач: %1\nПропущено: %2").arg(imported).arg(skipped));
//     saveTasks();
// }

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == allTasksView) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                return true; // Игнорируем Enter для таблицы задач
            }
        }
        if (event->type() == QEvent::MouseButtonDblClick) {
            return true; // Игнорируем двойной клик мыши по таблице задач
        }
    }
    if (obj == timeSlotsTable->viewport() && event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint pos = mouseEvent->pos();
        QModelIndex idx = timeSlotsTable->indexAt(pos);
        int row = idx.row();
        if (row >= 0) {
            QDate date = dateFilterEdit->date();
            int hour = row;
            int yInRow = pos.y() - timeSlotsTable->rowViewportPosition(row);
            int rowHeight = timeSlotsTable->rowHeight(row);
            double frac = static_cast<double>(yInRow) / rowHeight;
            int minute = static_cast<int>(frac * 60);
            minute = (minute / 5) * 5;
            QTime startTime(hour, minute);
            QTime endTime = startTime.addSecs(60 * 60); // по умолчанию 1 час
            if (endTime.hour() > 23) endTime.setHMS(23, 59, 0); // не выходить за пределы суток
            QDateTime startDT(date, startTime);
            QDateTime endDT(date, endTime);
            TaskDialog dialog(m_dataManager, this, Task("", "Работа", startDT, endDT, "В работе", ""));
            if (dialog.exec() == QDialog::Accepted) {
                Task task = dialog.getTask();
                taskModel->addTask(task);
                if (overlay) overlay->ignoreNextClick();
                refreshAllViews();
                saveTasks();
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (allTasksView) {
        allTasksView->resizeRowsToContents();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveTasks();
    event->accept();
}

void MainWindow::saveTasks()
{
    if (!taskModel->saveTasks()) {
        QMessageBox::warning(this, "Ошибка сохранения", "Не удалось сохранить задачи.");
    }
}

void MainWindow::loadTasks()
{
    if (!taskModel->loadTasks()) {
        QMessageBox::information(this, "Загрузка задач", "Файл с задачами не найден. Будет создан новый.");
    }
}

void MainWindow::checkForOverdueTasks()
{
    QDateTime now = QDateTime::currentDateTime();
    bool changed = false;
    for (int i = 0; i < taskModel->rowCount(); ++i) {
        Task task = taskModel->getTask(i);

        if (task.isProjectTask() && task.dueDateTime() < now &&
            (task.status() == "Не начато" || task.status() == "В процессе")) {

            task.setStatus("Просрочено");
            taskModel->updateTask(i, task);
            changed = true;
        }
    }

    if (changed) {
        refreshAllViews();
        saveTasks();
    }
}
