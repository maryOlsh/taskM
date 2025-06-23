#include "namecolordialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QColorDialog>
#include <QMessageBox>

NameColorDialog::NameColorDialog(const QString& title, QWidget *parent)
    : QDialog(parent), m_selectedColor(Qt::gray)
{
    setWindowTitle(title);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Введите название");

    m_colorButton = new QPushButton("Выбрать цвет", this);
    connect(m_colorButton, &QPushButton::clicked, this, &NameColorDialog::selectColor);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this](){
        if(m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Название не может быть пустым.");
            return;
        }
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(new QLabel("Название:"));
    mainLayout->addWidget(m_nameEdit);
    mainLayout->addWidget(m_colorButton);
    mainLayout->addWidget(buttonBox);

    m_colorButton->setStyleSheet(QString("background-color: %1;").arg(m_selectedColor.name()));
}

void NameColorDialog::selectColor()
{
    QColor color = QColorDialog::getColor(m_selectedColor, this, "Выберите цвет");
    if (color.isValid()) {
        m_selectedColor = color;
        m_colorButton->setStyleSheet(QString("background-color: %1;").arg(m_selectedColor.name()));
    }
}

QString NameColorDialog::name() const { return m_nameEdit->text().trimmed(); }
QColor NameColorDialog::color() const { return m_selectedColor; }
