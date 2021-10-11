#include "mainwindow.h"
#include "ui_mainwindow.h"
#include"logindialog.h"
#include"fileexplorer.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    sessionTab=ui->sessionTab;
    sessionTab->addTab(new FileExplorer(this),"session1");
    sessionTab->addTab(new FileExplorer(this),"session2");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_loginButton_clicked()
{
    LoginDialog* dialog=new LoginDialog(this);
    dialog->exec();
    sessionTab->removeTab(1);
}

