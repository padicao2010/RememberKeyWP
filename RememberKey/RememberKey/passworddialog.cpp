#include "passworddialog.h"
#include "ui_passworddialog.h"

#include <QMessageBox>

PasswordDialog::PasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordDialog)
{
    ui->setupUi(this);
}

PasswordDialog::~PasswordDialog()
{
    delete ui;
}

void PasswordDialog::on_buttonBox_accepted()
{
    int count = ui->countInput->value();
    bool capital = ui->capitalCheck->isChecked();
    bool small = ui->smallCheck->isChecked();
    bool digit = ui->digitCheck->isChecked();
    if(!capital && !small && !digit) {
        QMessageBox::warning(this, tr("Operation Error"),  tr("Can't generate password without capital letters, small letters and digits"));
        return;
    }

    this->accept();
    emit generatePassword(count, capital, small, digit);
}
