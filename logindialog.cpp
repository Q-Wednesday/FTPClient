#include "logindialog.h"
#include "ui_logindialog.h"
#include <QtDebug>
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
