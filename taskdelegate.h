/**
 * @file taskdelegate.h
 * @brief Делегат для кастомного отображения задач в представлении.
 */

#ifndef TASKDELEGATE_H
#define TASKDELEGATE_H

#include <QStyledItemDelegate>

/**
 * @class TaskDelegate
 * @brief Делегат для отрисовки задач в таблице/списке.
 */
class TaskDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TaskDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    virtual ~TaskDelegate() = default;
};

#endif // TASKDELEGATE_H
