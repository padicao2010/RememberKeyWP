#include "editdialog.h"
#include "ui_editdialog.h"

#include <QMessageBox>
#include <QCryptographicHash>
#include <QDateTime>
#include "passworddialog.h"

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

void EditDialog::generate_password(int count, bool capital, bool small, bool digits)
{
    qsrand(QDateTime::currentMSecsSinceEpoch());
    int remain = count;
    QString password;
    while(remain > 0) {
        QByteArray sampleArray;
        for(int i = 0; i < 256; i++) {
            sampleArray.append((char)(qrand() % 256));
        }
        QByteArray destArray = QCryptographicHash::hash(sampleArray, QCryptographicHash::Md5).toBase64();
        for(auto iter = destArray.cbegin(); iter != destArray.cend(); iter++) {
            char c = *iter;
            if((capital && c >= 'A'  && c <= 'Z') ||
                    (small && c >= 'a' && c <= 'z') ||
                    (digits && c >= '0' && c <= '9')) {
                password.append(c);
                remain--;
                if(remain == 0)  break;
            }
        }
    }
    ui->passwordEdit->setText(password);
    ui->passwordAgainEdit->setText(password);
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

void EditDialog::on_showPassButton_stateChanged(int state)
{
    if(state == Qt::Checked) {
        ui->passwordEdit->setEchoMode(QLineEdit::Normal);
        ui->passwordAgainEdit->setEchoMode(QLineEdit::Normal);
    } else {
        ui->passwordEdit->setEchoMode(QLineEdit::Password);
        ui->passwordAgainEdit->setEchoMode(QLineEdit::Password);
    }
}

void EditDialog::on_generateButton_clicked()
{
    PasswordDialog *dialog = new PasswordDialog(this);
    dialog->setModal(true);
    connect(dialog, &PasswordDialog::finished, [=](int state) {
        dialog->deleteLater();
    });
    connect(dialog, &PasswordDialog::generatePassword, this, &EditDialog::generate_password);
    dialog->show();
}
