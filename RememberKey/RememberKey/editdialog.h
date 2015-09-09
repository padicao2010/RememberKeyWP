#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include <QDialog>
#include "keyinfo.h"

namespace Ui {
class EditDialog;
}

class EditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditDialog(QWidget *parent = 0);
    ~EditDialog();

    void setInfo(bool add, const KeyInfo *key);

public slots:
    void error_happened(const QString &errMsg);
    void generate_password(int count, bool capital, bool small, bool digits);

signals:
    void addInputReady(const KeyInfo &key);
    void updateInputReady(const KeyInfo &old, const KeyInfo &key);

private slots:
    void on_saveButton_clicked();

    void on_showPassButton_stateChanged(int arg1);

    void on_generateButton_clicked();

private:
    Ui::EditDialog *ui;

    KeyInfo currentKey;
    KeyInfo oldKey;
    bool toAdd;
};

#endif // EDITDIALOG_H
