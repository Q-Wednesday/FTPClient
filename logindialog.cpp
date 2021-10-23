#include "logindialog.h"
#include "ui_logindialog.h"
#include"clientcore.h"
LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    disconnect(client,nullptr,this,nullptr);
    delete ui;
}

void LoginDialog::on_buttonBox_accepted()
{
    client=new ClientCore(ui->hostNameEdit->text(),ui->portSelection->value());
    client->userLogin(ui->userNameEdit->text(),ui->passwordEdit->text());
    connect(client,&ClientCore::initSuccess,this,[this](bool flag){
            emit LoginSuccess(client);
    });
    client->connectServer();
}

