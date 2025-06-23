/**
 * @file taskscheduleoverlay.cpp
 * @brief Реализация наложения расписания задач поверх QTableWidget.
 */
#include "taskslot.h"
#include "task.h"
#include "taskscheduleoverlay.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>
#include <QVector>
#include <QHelpEvent>
#include <QScrollBar>
#include <algorithm>
#include <QMenu>
#include <QPoint>
#include "taskfilterproxymodel.h"
#include "taskmodel.h"
#include <QDebug>
#include <QSet>
#include <QApplication>
#include <numeric>
#include "customdatamanager.h"
#include <QTimer>
#include <QTime>

/**
 * @struct PositionedTask
 * @brief Структура для хранения позиции и размера задачи на оверлее.
 */
struct PositionedTask {
    Task task;
    int top;
    int height;
    int left;
    int width;
    int column;
    int columnsCount;
};

/**
 * @brief Вычисляет позиции задач для отображения на оверлее.
 * @param tasks Список задач.
 * @param table Таблица для отображения.
 * @param minWidth Минимальная ширина колонки.
 * @param overlayWidth Ширина оверлея.
 * @return Вектор позиций задач.
 */
static QVector<PositionedTask> calculateTaskPositions(
    const QVector<Task>& tasks, QTableWidget* table, int minWidth, int overlayWidth)
{
    const int totalMinutes = 24 * 60;

    // 1. Сортируем задачи по времени начала
    QVector<int> taskIndices(tasks.size());
    std::iota(taskIndices.begin(), taskIndices.end(), 0);
    std::sort(taskIndices.begin(), taskIndices.end(), [&](int a, int b) {
        if (tasks[a].startDateTime() != tasks[b].startDateTime()) {
            return tasks[a].startDateTime() < tasks[b].startDateTime();
        }
        return tasks[a].endDateTime() < tasks[b].endDateTime();
    });

    // 2. Распределяем задачи по колонкам
    QVector<int> taskColumns(tasks.size(), -1);
    QVector<QDateTime> columnEndTimes; // Хранит время окончания последней задачи в колонке

    for (int i : taskIndices) {
        if (!tasks[i].isProjectTask() || !tasks[i].startDateTime().isValid() || !tasks[i].endDateTime().isValid() || tasks[i].startDateTime() >= tasks[i].endDateTime()) {
            continue;
        }

        QDateTime currentTaskStart = tasks[i].startDateTime();
        int freeCol = -1;

        // Ищем первую свободную колонку
        for (int col = 0; col < columnEndTimes.size(); ++col) {
            if (currentTaskStart >= columnEndTimes[col]) {
                freeCol = col;
                break;
            }
        }

        if (freeCol == -1) {
            // Если свободной колонки нет, создаем новую
            freeCol = columnEndTimes.size();
            columnEndTimes.append(QDateTime());
        }

        taskColumns[i] = freeCol;
        columnEndTimes[freeCol] = tasks[i].endDateTime();
    }

    int globalMaxColumns = columnEndTimes.size();

    // 3. Рассчитываем позиции и размеры задач
    QVector<PositionedTask> result;
    int viewportWidth = overlayWidth;
    int totalWidth = std::max(viewportWidth, globalMaxColumns * minWidth);
    int columnWidth = (globalMaxColumns > 0) ? (totalWidth / globalMaxColumns) : totalWidth;

    for (int i = 0; i < tasks.size(); ++i) {
        if (!tasks[i].isProjectTask() || taskColumns[i] == -1) continue;

        if (!tasks[i].startDateTime().isValid() || !tasks[i].endDateTime().isValid()) continue;
        if (tasks[i].startDateTime() >= tasks[i].endDateTime()) continue;

        int s = tasks[i].startDateTime().time().hour() * 60 +
                tasks[i].startDateTime().time().minute();
        int e = tasks[i].endDateTime().time().hour() * 60 +
                tasks[i].endDateTime().time().minute();
        s = std::max(0, s);
        e = std::min(totalMinutes, e);
        if (e <= s) e = s + 1;

        // Рассчитываем вертикальную позицию
        int startRow = s / 60;
        int endRow = (e - 1) / 60;

        if (startRow < 0 || startRow >= table->rowCount() ||
            endRow < 0 || endRow >= table->rowCount()) {
            continue;
        }

        int yStart = table->rowViewportPosition(startRow) +
                     static_cast<int>((s % 60) * (table->rowHeight(startRow) / 60.0));

        int yEnd;
        if (e == totalMinutes) {
            yEnd = table->rowViewportPosition(23) + table->rowHeight(23);
        } else {
            int endMinute = e % 60;
            if (endMinute == 0) {
                yEnd = table->rowViewportPosition(endRow) + table->rowHeight(endRow);
            } else {
                yEnd = table->rowViewportPosition(endRow) +
                       static_cast<int>(endMinute * (table->rowHeight(endRow) / 60.0));
            }
        }

        // Корректировка для видимой области
        int visibleTop = std::max(0, yStart);
        int visibleBottom = std::min(table->viewport()->height(), yEnd);
        int visibleHeight = visibleBottom - visibleTop;

        // Пропускаем слишком маленькие задачи
        if (visibleHeight < 10) continue;

        // Рассчитываем горизонтальную позицию
        int col = taskColumns[i];
        int left = col * columnWidth;
        int width = columnWidth;

        // Гарантируем минимальную ширину
        if (width < minWidth) width = minWidth;

        result.append(PositionedTask{
            tasks[i], visibleTop, visibleHeight, left, width, col, globalMaxColumns
        });
    }

    return result;
}

// --- TaskScheduleOverlay implementation ---

/**
 * @class TaskScheduleOverlay
 * @brief Класс для отображения задач по времени поверх QTableWidget.
 */
TaskScheduleOverlay::TaskScheduleOverlay(QTableWidget *table, TaskModel *model, TaskFilterProxyModel *proxyModel, CustomDataManager *dataManager, QWidget *parent)
    : QWidget(parent), m_table(table), m_model(model), m_proxyModel(proxyModel), m_dataManager(dataManager)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMouseTracking(true);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);

    if (m_model) {
        connect(m_model, &QAbstractItemModel::rowsInserted, this, [this]() { updateOverlay(); });
        connect(m_model, &QAbstractItemModel::rowsRemoved, this, [this]() { updateOverlay(); });
        connect(m_model, &QAbstractItemModel::dataChanged, this, [this]() { updateOverlay(); });
    }
    if (m_table && m_table->viewport()) {
        setGeometry(m_table->viewport()->rect());
    }
    if (m_table && m_table->viewport()) {
        connect(m_table->verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
            setGeometry(m_table->viewport()->rect());
            updateOverlay();
        });
        connect(m_table->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
            setGeometry(m_table->viewport()->rect());
            updateOverlay();
        });
        connect(m_table->verticalHeader(), &QHeaderView::sectionResized, this, [this]() {
            setGeometry(m_table->viewport()->rect());
            updateOverlay();
        });
        connect(m_table->horizontalHeader(), &QHeaderView::sectionResized, this, [this]() {
            setGeometry(m_table->viewport()->rect());
            updateOverlay();
        });
    }
    m_selectedDate = QDate::currentDate();
    updateOverlay();

    m_timelineTimer = new QTimer(this);
    connect(m_timelineTimer, &QTimer::timeout, this, QOverload<>::of(&TaskScheduleOverlay::update));
    m_timelineTimer->start(60000); // Update every minute
}


void TaskScheduleOverlay::ignoreNextClick() {
    m_ignoreNextClick = true;
}
void TaskScheduleOverlay::updateOverlay() {
    recalculateRects();
    update();
}

void TaskScheduleOverlay::setSelectedDate(const QDate& date) {
    if (m_selectedDate != date) {
        m_selectedDate = date;
        updateOverlay();
    }
}

void TaskScheduleOverlay::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    if (m_table && m_table->viewport()) {
        setGeometry(m_table->viewport()->rect());
    }
    updateOverlay();
}

void TaskScheduleOverlay::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Отрисовка временной сетки
    int w = width();
    for (int hour = 0; hour < 24; ++hour) {
        int yHour = m_table->rowViewportPosition(hour);
        int rowHeight = m_table->rowHeight(hour);

        // Основные линии часов
        painter.setPen(QPen(QColor(180,180,180), 2));
        painter.drawLine(0, yHour, w, yHour);

        // Подписи часов (Убрано, так как есть в заголовке)
        // painter.setPen(QPen(QColor(200,200,200), 1));
        // painter.drawText(5, yHour + 15, QString("%1:00").arg(hour, 2, 10, QChar('0')));

        // Линии 15-минутных интервалов
        for (int m = 15; m < 60; m += 15) {
            int y = yHour + static_cast<int>((m / 60.0) * rowHeight);
            painter.setPen(QPen(QColor(100,100,100), 1, Qt::DashLine));
            painter.drawLine(0, y, w, y);
        }
    }



    // Отрисовка задач
    for (int i = 0; i < m_taskRects.size(); ++i) {
        const auto &rectTask = m_taskRects[i];
        
        QColor color = m_dataManager->getProjectColor(rectTask.task.projectType());

        // Эффект при наведении
        if (i == m_hoveredTaskIndex) {
            color = color.lighter(120);
            painter.setPen(QPen(QColor(220, 240, 255), 2));
        } else {
            painter.setPen(Qt::NoPen);
        }

        // Отрисовка закругленного прямоугольника
        painter.setBrush(color);
        painter.drawRoundedRect(rectTask.rect.adjusted(1,1,-1,-1), 6, 6);

        // Отрисовка текста
        QFont font = painter.font();
        font.setPointSize(9);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::white);

        // Многострочный текст с переносом
        QTextOption textOption;
        textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        textOption.setAlignment(Qt::AlignLeft | Qt::AlignTop);

        QRect textRect = rectTask.rect.adjusted(8, 4, -8, -4);
        painter.drawText(textRect, rectTask.task.title(), textOption);
    }
    // Отрисовка линии текущего времени
    if (m_selectedDate == QDate::currentDate()) {
        QTime now = QTime::currentTime();
        int hour = now.hour();
        int minute = now.minute();
        int yHour = m_table->rowViewportPosition(hour);
        int rowHeight = m_table->rowHeight(hour);
        int y = yHour + static_cast<int>((minute / 60.0) * rowHeight);

        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(0, y, w, y);
    }
}

void TaskScheduleOverlay::mouseMoveEvent(QMouseEvent *event) {
    m_lastMousePos = event->pos();
    int prevHovered = m_hoveredTaskIndex;
    m_hoveredTaskIndex = -1;

    for (int i = 0; i < m_taskRects.size(); ++i) {
        if (m_taskRects[i].rect.contains(event->pos())) {
            m_hoveredTaskIndex = i;
            setCursor(Qt::PointingHandCursor);
            break;
        }
    }

    if (m_hoveredTaskIndex == -1) {
        setCursor(Qt::ArrowCursor);
    }

    if (prevHovered != m_hoveredTaskIndex) {
        update();
    }
}

void TaskScheduleOverlay::mousePressEvent(QMouseEvent *event) {
    if (m_ignoreNextClick) {
        m_ignoreNextClick = false;
        event->accept();
        return;
    }

    for (int i = 0; i < m_taskRects.size(); ++i) {
        if (m_taskRects[i].rect.contains(event->pos())) {
            if (event->button() == Qt::LeftButton) {
                const Task& task = m_taskRects[i].task;
                QDateTime adjustedTaskEnd = QDateTime(m_selectedDate, task.endDateTime().time());

                if (adjustedTaskEnd < QDateTime::currentDateTime()) {
                    return; // Не разрешать редактирование прошедших задач
                }

                int proxyRow = -1;
                if (m_proxyModel) {
                    for (int row = 0; row < m_proxyModel->rowCount(); ++row) {
                        QModelIndex idx = m_proxyModel->index(row, 0);
                        Task t = idx.data(TaskModel::FullTaskRole).value<Task>();
                        if (t.uid() == m_taskRects[i].task.uid()) {
                            proxyRow = row;
                            break;
                        }
                    }
                }
                emit editTaskRequested(m_taskRects[i].task, proxyRow);
            }
            return;
        }
    }
}

bool TaskScheduleOverlay::event(QEvent *event) {
    if (event->type() == QEvent::ToolTip && m_hoveredTaskIndex != -1) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);
        showTaskTooltip(helpEvent->globalPos(), m_taskRects[m_hoveredTaskIndex].task);
        return true;
    }
    return QWidget::event(event);
}

void TaskScheduleOverlay::mouseDoubleClickEvent(QMouseEvent *event) {
    if (!m_table || !m_proxyModel) return;

    for (const auto& rectTask : m_taskRects) {
        if (rectTask.rect.contains(event->pos())) {
            return;
        }
    }

    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QMouseEvent ev(event->type(), event->position(), event->scenePosition(),
                   event->globalPosition(), event->button(), event->buttons(),
                   event->modifiers());
    QApplication::sendEvent(parentWidget(), &ev);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    QWidget::mouseDoubleClickEvent(event);
}

void TaskScheduleOverlay::recalculateRects() {
    m_taskRects.clear();
    m_overflowTasks.clear();

    if (!m_proxyModel || !m_table) {
        return;
    }

    QVector<Task> tasksForDay;
    QVector<Task> filteredTasks;

    QAbstractItemModel* sourceModel = m_proxyModel->sourceModel();
    if (!sourceModel) return;

    for (int i = 0; i < sourceModel->rowCount(); ++i) {
        if (m_proxyModel->filterAcceptsRow_IgnoreDeadline(i, QModelIndex())) {
            QModelIndex sourceIndex = sourceModel->index(i, 0);
            Task task = sourceIndex.data(TaskModel::FullTaskRole).value<Task>();
            filteredTasks.append(task);
        }
    }

    // Фильтрация задач для выбранной даты
    for (const Task& task : filteredTasks) {
        if (!task.isProjectTask()) continue;
        if (!task.startDateTime().isValid() || !task.endDateTime().isValid()) continue;
        if (task.startDateTime() >= task.endDateTime()) continue;

        QDate startDate = task.startDateTime().date();
        QDate endDate = task.endDateTime().date();

        if (m_selectedDate.isValid() &&
            !(startDate <= m_selectedDate && m_selectedDate <= endDate)) {
            continue;
        }

        // Корректировка времени для выбранной даты
        QDateTime start = task.startDateTime();
        QDateTime end = task.endDateTime();

        if (startDate != m_selectedDate) {
            start = QDateTime(m_selectedDate, QTime(0, 0));
        }
        if (endDate != m_selectedDate) {
            end = QDateTime(m_selectedDate, QTime(23, 59));
        }

        Task adjustedTask = task;
        adjustedTask.setStartDateTime(start);
        adjustedTask.setEndDateTime(end);
        tasksForDay.push_back(adjustedTask);
    }

    int minWidth = 60;
    int viewportWidth = m_table->viewport()->width();
    auto positionedTasks = calculateTaskPositions(tasksForDay, m_table, minWidth, viewportWidth);

    // Формируем видимые прямоугольники
    for (const auto& pt : positionedTasks) {
        QRect rect(pt.left, pt.top, pt.width, pt.height);
        m_taskRects.append({rect, pt.task});
    }

    // Рассчитываем общую ширину
    int maxColAll = 1;
    for (const auto& pt : positionedTasks) {
        maxColAll = std::max(maxColAll, pt.columnsCount);
    }
    int totalWidth = std::max(viewportWidth, maxColAll * minWidth);
    resize(totalWidth, height());
}


void TaskScheduleOverlay::showTaskTooltip(const QPoint &globalPos, const Task &task) {
    QString text = QString("<b>%1</b><br>"
                           "Проект: %2<br>"
                           "Время: %3 - %4<br>"
                           "Статус: %5")
                       .arg(task.title())
                       .arg(task.projectType())
                       .arg(task.startDateTime().toString("dd.MM.yyyy HH:mm"))
                       .arg(task.endDateTime().toString("dd.MM.yyyy HH:mm"))
                       .arg(task.status());
    QToolTip::showText(globalPos, text, this);
}

void TaskScheduleOverlay::showOverflowPopup(const QPoint &pos, const QList<Task> &tasks) {
    QMenu menu(this);
    for (const auto& task : tasks) {
        QString text = QString("%1 (%2, %3 - %4)")
        .arg(task.title())
            .arg(task.projectType())
            .arg(task.startDateTime().toString("HH:mm"))
            .arg(task.endDateTime().toString("HH:mm"));
        QAction *action = menu.addAction(text);
        connect(action, &QAction::triggered, this, [this, task]() {
            emit editTaskRequested(task, -1);
        });
    }
    menu.exec(pos);
}
