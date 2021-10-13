#include "fileexplorer.h"
#include "ui_fileexplorer.h"

FileExplorer::FileExplorer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileExplorer)
{
    ui->setupUi(this);
}

FileExplorer::~FileExplorer()
{
    delete ui;
}

void FileExplorer::on_pushButton_clicked()
{
    //client=new ClientCore("localhost",21,this);
    qDebug()<<client->getState();
    client->commandLIST();
}

void FileExplorer::bindClient(ClientCore* clientLogin){
    client=clientLogin;
}
