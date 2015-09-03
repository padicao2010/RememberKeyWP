#include "newdbdialog.h"
#include "ui_newdbdialog.h"

NewDBDialog::NewDBDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewDBDialog)
{
    ui->setupUi(this);
}

NewDBDialog::~NewDBDialog()
{
    delete ui;
}
