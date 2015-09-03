#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class CreateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateDialog(QWidget *parent = 0);
    ~CreateDialog();

signals:
    void inputCreateReady(const QString &filepath, const QString &password, bool onedrive);

public slots:
    void error_happened(const QString &errMag);

private slots:
    void on_filepathButton_clicked();
    void on_buttonBox_accepted();

private:
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
