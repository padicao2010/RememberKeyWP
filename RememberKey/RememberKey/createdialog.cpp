#include "createdialog.h"
#include "ui_createdialog.h"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
CreateDialog::CreateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

CreateDialog::~CreateDialog()
{
    delete ui;
}

void CreateDialog::error_happened(const QString &errMsg)
{
    QMessageBox::information(this, tr("Login Error"), errMsg);
}

void CreateDialog::on_filepathButton_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this,
            tr("Create a new SQLite database file"),
            "", tr("RememberKey Config File(*.rkini)"));
    if(filename.isEmpty()) {
        return;
    }

    if(!filename.endsWith(".rkini")) {
        filename += ".rkini";
    }

    if(QFile(filename).exists()) {
        QMessageBox::warning(this, tr("Choose file error"), tr("%1 is existed").arg(filename));
        return;
    }

    if(QFile(filename + ".db").exists()) {
        QMessageBox::warning(this, tr("Choose file error"), tr("%1.db is existed").arg(filename));
        return;
    }

    ui->filepathEdit->setText(filename);
}

void CreateDialog::on_buttonBox_accepted()
{
    QString filepath = ui->filepathEdit->text();
    if(filepath.isEmpty()) {
        QMessageBox::information(this, tr(""), tr("File path should not be null"));
        return;
    }
    QString password = ui->passswdEdit->text();
    if(password.isEmpty()) {
        QMessageBox::information(this, tr(""), tr("Password should not be null"));
        return;
    }
    if(password != ui->passwdAgainEdit->text()) {
        QMessageBox::information(this, tr(""), tr("Passwords are not the same"));
        return;
    }

    ui->passswdEdit->setText("");
    ui->passwdAgainEdit->setText("");
    bool onedrive = ui->onedriveCheck->isChecked();
    emit inputCreateReady(filepath, password, onedrive);
}
