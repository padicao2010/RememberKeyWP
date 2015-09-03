#include "editdialog.h"
#include "ui_editdialog.h"

#include <QMessageBox>

EditDialog::EditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDialog)
{
    ui->setupUi(this);
    currentKey.setId(-1);
}

EditDialog::~EditDialog()
{
    delete ui;
}

void EditDialog::setInfo(bool add, const KeyInfo *key)
{
    toAdd = add;
    if(add) {
        oldKey.reset();
        currentKey.reset();
    } else {
        oldKey = *key;
        currentKey = *key;
    }

    ui->nameEdit->setText(currentKey.getName());
    ui->siteEdit->setText(currentKey.getSite());
    ui->usernameEdit->setText(currentKey.getUsername());
    ui->passwordEdit->setText(currentKey.getPassword());
    ui->passwordAgainEdit->setText(currentKey.getPassword());
    ui->notesEdit->setPlainText(currentKey.getNotes());
}

void EditDialog::error_happened(const QString &errMsg)
{
    QMessageBox::information(this, tr("Edit Error"), errMsg);
}

void EditDialog::on_saveButton_clicked()
{
    currentKey.setName(ui->nameEdit->text());
    currentKey.setSite(ui->siteEdit->text());
    currentKey.setUsername(ui->usernameEdit->text());
    currentKey.setPassword(ui->passwordEdit->text());
    currentKey.setNotes(ui->notesEdit->toPlainText());

    if(currentKey.getName().isEmpty()) {
        QMessageBox::information(this, tr(""), tr("Name should not be null"));
        return;
    }

    if(currentKey.getPassword().isEmpty()) {
        QMessageBox::information(this, tr(""), tr("Password should not be null"));
        return;
    }

    if(currentKey.getPassword() != ui->passwordAgainEdit->text()) {
        QMessageBox::information(this, tr(""), tr("Password mismatch"));
        return;
    }

    if(toAdd) {
        emit addInputReady(currentKey);
    } else {
        emit updateInputReady(oldKey, currentKey);
    }
}
