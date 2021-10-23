#include "mainwindow.h"
#include "ui_mainwindow.h"

#include"fileexplorer.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    sessionTab=ui->sessionTab;
    connect(sessionTab,&QTabWidget::tabCloseRequested,this,&MainWindow::closeSession);//点击tab的叉叉时请求关闭连接和tab
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_loginButton_clicked()
{
    dialog=new LoginDialog(this);
    connect(dialog,&LoginDialog::LoginSuccess,this,&MainWindow::newSession);
    dialog->exec();
    //sessionTab->removeTab(1);
}

void MainWindow::newSession(ClientCore* client){
    FileExplorer* explorer=new FileExplorer(this);
    explorer->bindClient(client);
    sessionTab->addTab(explorer,client->getHostName());
    dialog->deleteLater();
}

void MainWindow::closeSession(int index){
    //关闭连接和tab页。
    FileExplorer* fileExplorer=qobject_cast<FileExplorer*>(sessionTab->widget(index));
    connect(fileExplorer,&FileExplorer::sessionClosed,[this,index,fileExplorer]{
        qDebug()<<"close tab:"<<index;
        sessionTab->removeTab(index);
        fileExplorer->deleteLater();
    });
    fileExplorer->closeSession();
}
