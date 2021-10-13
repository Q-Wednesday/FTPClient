#include "logindialog.h"
#include "ui_logindialog.h"
#include <QtDebug>
#include"clientcore.h"
LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    qDebug()<<"destroy login dialog";
    delete ui;
}

void LoginDialog::on_buttonBox_accepted()
{
    ClientCore* client=new ClientCore(ui->hostNameEdit->text(),ui->portSelection->value());
    if(client->connectServer()){
        qDebug()<<"success";
    }
    client->userLogin(ui->userNameEdit->text(),ui->passwordEdit->text());
    connect(client,&ClientCore::initSuccess,[this,client](bool flag){
        if(flag){
            emit LoginSuccess(client);
        }else{
            delete client;
            emit LoginSuccess(nullptr);
        }
    });   //TODO: show Error
}

