#include "namedialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>

NameDialog::NameDialog(const QString& title, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(title);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Введите название");

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
    mainLayout->addWidget(buttonBox);
}

QString NameDialog::name() const { return m_nameEdit->text().trimmed(); }
