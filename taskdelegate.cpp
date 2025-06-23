/**
 * @file taskdelegate.cpp
 * @brief Реализация делегата для отображения задач.
 */
#include "taskdelegate.h"
#include "taskmodel.h"
#include <QPainter>
#include <QApplication>
#include <QDate>
#include <QString>
#include <QDateTime>
#include <QFontMetrics>
#include "task.h"

TaskDelegate::TaskDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

void TaskDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    if (!index.isValid()) return;

    painter->save();

    // Получение данных
    Task task = index.data(TaskModel::FullTaskRole).value<Task>();
    QString title = task.title();
    QString project = task.projectType();
    QString status = task.status();
    QString dateStr = Task::formatDate(task);


    // Отрисовка фона
    QRect rect = option.rect;
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);

    // Цвет фона в зависимости от статуса
    QColor bgColor;
    if (status == "Выполнено") bgColor = QColor(200, 255, 200);
    else if (status == "В процессе") bgColor = QColor(255, 255, 150);
    else bgColor = QColor(240, 240, 240);

    painter->setBrush(bgColor);
    painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 5, 5);

    // Отрисовка текста
    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setBold(true);
    painter->setFont(font);
    if (index.column() == TaskModel::TitleColumn) {
        painter->drawText(rect.adjusted(10, 5, -10, -20), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, title);
    } else if (index.column() == TaskModel::DateColumn) {
        painter->drawText(rect.adjusted(10, 25, -10, -5), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, dateStr);
    } else {
        font.setBold(false);
        painter->setFont(font);
        painter->setPen(Qt::darkGray);
        QString details = QString("%1 | %2 | %3")
                              .arg(dateStr)
                              .arg(project)
                              .arg(status);
        painter->drawText(rect.adjusted(10, 25, -10, -5), Qt::AlignLeft | Qt::AlignTop, details);
    }

    // Выделение выбранного элемента
    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(Qt::blue, 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1), 5, 5);
    }

    painter->restore();
}

QSize TaskDelegate::sizeHint(const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    Task task = index.data(TaskModel::FullTaskRole).value<Task>();
    QString text;
    if (index.column() == TaskModel::TitleColumn) {
        text = task.title();
    } else if (index.column() == TaskModel::DateColumn) {
        text = Task::formatDate(task);
    } else {
        text = index.data(Qt::DisplayRole).toString();
    }
    QFont font = option.font;
    QFontMetrics fm(font);
    int width = option.rect.width() > 0 ? option.rect.width() : 120;
    QRect textRect = fm.boundingRect(QRect(0, 0, width - 20, 1000), Qt::TextWordWrap, text);
    int height = textRect.height() + 20;
    int minWidth = 120;
    if (index.column() == TaskModel::TitleColumn)
        minWidth = 400;
    if (index.column() == TaskModel::DateColumn)
        minWidth = 150;
    return QSize(minWidth, std::max(height, 40));
}
