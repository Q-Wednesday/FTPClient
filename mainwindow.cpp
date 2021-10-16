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
    connect(sessionTab,&QTabWidget::tabCloseRequested,this,&MainWindow::closeSession);
    //sessionTab->addTab(new FileExplorer(this),"session1");
    //sessionTab->addTab(new FileExplorer(this),"session2");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_loginButton_clicked()
{
    LoginDialog* dialog=new LoginDialog(this);
    connect(dialog,&LoginDialog::LoginSuccess,this,&MainWindow::newSession);
    dialog->exec();
    //sessionTab->removeTab(1);
}

void MainWindow::newSession(ClientCore* client){
    FileExplorer* explorer=new FileExplorer(this);
    explorer->bindClient(client);
    sessionTab->addTab(explorer,client->getHostName());
}

void MainWindow::closeSession(int index){

    FileExplorer* fileExplorer=qobject_cast<FileExplorer*>(sessionTab->widget(index));
    connect(fileExplorer,&FileExplorer::sessionClosed,[this,index]{
        sessionTab->removeTab(index);
    });
    fileExplorer->closeSession();
}
