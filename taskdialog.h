/**
 * @file taskdialog.h
 * @brief Диалог для создания и редактирования задач.
 */

#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include "task.h"
#include <QTimeEdit>
#include <QCheckBox>
#include <QDateEdit>

class QLineEdit;
class QDateTimeEdit;
class QComboBox;
class QTextEdit;
class CustomDataManager;

/**
 * @class TaskDialog
 * @brief Диалоговое окно для создания и редактирования задач.
 */
class TaskDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief Конструктор TaskDialog.
     * @param dataManager Менеджер пользовательских данных.
     * @param parent Родительский виджет.
     * @param task Исходная задача (по умолчанию пустая).
     */
    explicit TaskDialog(CustomDataManager *dataManager, QWidget *parent = nullptr, const Task &task = Task());
    /**
     * @brief Получить задачу из диалога.
     * @return Задача.
     */
    Task getTask() const;

    /**
     * @brief Установить дату и время дедлайна.
     * @param dateTime Дата и время.
     */
    void setDueDateTime(const QDateTime &dateTime);
    /**
     * @brief Переопределённый exec для показа диалога.
     * @return Код завершения.
     */
    int exec() override;

signals:
    /**
     * @brief Сигнал об удалении задачи.
     */
    void taskDeleted();

private slots:
    /**
     * @brief Проверка и подтверждение данных при нажатии OK.
     */
    void validateAndAccept();
    /**
     * @brief Слот для переключения режима задачи по времени.
     * @param checked Включено/выключено.
     */
    void onTimedTaskToggled(bool checked);

private:
    /**
     * @brief Заполняет комбобоксы значениями из менеджера данных.
     */
    void populateCombos();

    QLineEdit *titleEdit;
    QComboBox *projectTypeCombo;
    QComboBox *statusCombo;
    QComboBox *priorityCombo;
    QTextEdit *descriptionEdit;
    QCheckBox *isTimedTaskCheck;
    QDateEdit *dateEdit;
    QTimeEdit *startTimeEdit;
    QTimeEdit *endTimeEdit;
    CustomDataManager *m_dataManager;
    Task m_originalTask;
};

#endif // TASKDIALOG_H
