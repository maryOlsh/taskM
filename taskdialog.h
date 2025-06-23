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

class TaskDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TaskDialog(CustomDataManager *dataManager, QWidget *parent = nullptr, const Task &task = Task());
    Task getTask() const;

    void setDueDateTime(const QDateTime &dateTime);
    int exec() override;

signals:
    void taskDeleted();

private slots:
    void validateAndAccept();
    void onTimedTaskToggled(bool checked);

private:
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
