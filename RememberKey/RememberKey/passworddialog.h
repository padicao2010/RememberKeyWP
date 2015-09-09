#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class PasswordDialog;
}

class PasswordDialog : public QDialog
{
    Q_OBJECT

signals:
    void generatePassword(int count, bool capital, bool small, bool digits);

public:
    explicit PasswordDialog(QWidget *parent = 0);
    ~PasswordDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::PasswordDialog *ui;
};

#endif // PASSWORDDIALOG_H
