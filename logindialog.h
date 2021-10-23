#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include"clientcore.h"
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
signals:
    void LoginSuccess(ClientCore*);//失败则发送nullptr

private slots:
    void on_buttonBox_accepted();

private:
    Ui::LoginDialog *ui;
    ClientCore* client;
};

#endif // LOGINDIALOG_H
