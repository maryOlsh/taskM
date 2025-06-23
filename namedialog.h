#ifndef NAMEDIALOG_H
#define NAMEDIALOG_H

#include <QDialog>

class QLineEdit;

class NameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NameDialog(const QString& title, QWidget *parent = nullptr);

    QString name() const;

private:
    QLineEdit *m_nameEdit;
};

#endif // NAMEDIALOG_H
