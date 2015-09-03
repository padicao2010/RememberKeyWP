#ifndef NEWDBDIALOG_H
#define NEWDBDIALOG_H

#include <QDialog>

namespace Ui {
class NewDBDialog;
}

class NewDBDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewDBDialog(QWidget *parent = 0);
    ~NewDBDialog();

private:
    Ui::NewDBDialog *ui;
};

#endif // NEWDBDIALOG_H
