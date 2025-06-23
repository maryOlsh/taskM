#ifndef NAMECOLORDIALOG_H
#define NAMECOLORDIALOG_H

#include <QDialog>
#include <QColor>

class QLineEdit;
class QPushButton;

class NameColorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NameColorDialog(const QString& title, QWidget *parent = nullptr);

    QString name() const;
    QColor color() const;

private slots:
    void selectColor();

private:
    QLineEdit *m_nameEdit;
    QPushButton *m_colorButton;
    QColor m_selectedColor;
};

#endif // NAMECOLORDIALOG_H
