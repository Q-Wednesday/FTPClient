#include "fileexplorer.h"
#include "ui_fileexplorer.h"
#include<QIcon>
#include<QDir>
FileExplorer::FileExplorer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileExplorer)
{
    ui->setupUi(this);
    remoteFileInfo=ui->remoteFileInfo;
    connect(ui->remoteDir,&QListWidget::itemClicked,this,&FileExplorer::changeRemoteWorkDir);
    showLocalFileInfo();
    connect(ui->localDir,&QListWidget::itemClicked,this,&FileExplorer::changeLocalWorkDir);

}

FileExplorer::~FileExplorer()
{
    delete ui;
}

void FileExplorer::on_pushButton_clicked()
{
    //client=new ClientCore("localhost",21,this);
    qDebug()<<client->getState();
    //client->commandLIST();
    client->commandPWD();


}

void FileExplorer::bindClient(ClientCore* clientLogin){
    if(client==nullptr)return;
    client=clientLogin;
    connect(client,&ClientCore::fileInfoGeted,this,&FileExplorer::showRemoteFileInfo);
    connect(client,&ClientCore::pwdGeted,this,&FileExplorer::showRemoteWorkDir);
    connect(client,&ClientCore::serverReponse,[this](QString response){
        ui->serverResponse->append(response);
    });
}

void FileExplorer::showRemoteFileInfo(QString infoReceived){
    remoteFileInfo->clear();
    auto infos=infoReceived.split('\n');
    //实现解析文件信息
    for(auto info:infos){
        if(info.isEmpty())continue;
        if(info[0]=='-'){
            QListWidgetItem* temp=new QListWidgetItem(info.split(' ').last());
            temp->setIcon(QIcon(":/icons/file"));

            remoteFileInfo->addItem(temp);
        }else if(info[0]=='d'){
            QListWidgetItem* temp=new QListWidgetItem(info.split(' ').last());
            temp->setIcon(QIcon(":/icons/dir"));
            remoteFileInfo->addItem(temp);
        }else if(info[0]=='l'){
            QListWidgetItem* temp=new QListWidgetItem(info.split(' ').last());
            temp->setIcon(QIcon(":/icons/link"));
            remoteFileInfo->addItem(temp);
        }else{
            //其他类型暂时不支持解析
            continue;
        }
    }
}
void FileExplorer::showRemoteWorkDir(QString workdir){
    ui->remoteDir->clear();
    auto dirs=workdir.split('/');
    QString prefix="";
    dirs.pop_front();//分隔开的时候最前面一个不要.所有pwd是没有后缀/的
    for(auto dir:dirs){
        QListWidgetItem* temp=new QListWidgetItem(QString("/%1").arg(dir));
        prefix+=dir+'/';
        temp->setData(1,prefix);
        ui->remoteDir->addItem(temp);
    }
    client->commandLIST();

}

void FileExplorer::changeRemoteWorkDir(QListWidgetItem* item){
    client->commandCWD(item->data(1).toString());

}
void FileExplorer::showLocalFileInfo(QString localPath){

    if(localPath.isEmpty())
        localPath=QDir::currentPath();
qDebug()<<"localPath:"<<localPath;
    auto dirs=localPath.split('/');
    QString prefix="";
    if(localPath[0]=='/'){
        //mac或linux下
        dirs.pop_front();
    }
    ui->localDir->clear();
    for(auto dir:dirs){
        QListWidgetItem* temp=new QListWidgetItem(QString("/%1").arg(dir));
        qDebug()<<"dir:"<<dir;
        prefix+='/'+dir;
        qDebug()<<"prefix:"<<prefix;
        temp->setData(1,prefix);
        ui->localDir->addItem(temp);
    }

    QDir localDir=QDir(localPath);
    ui->localFileInfo->clear();
    for(auto fileInfo:localDir.entryInfoList()){
        if(fileInfo.fileName()=="." || fileInfo.fileName()=="..")continue;
        QListWidgetItem* temp=new QListWidgetItem(fileInfo.fileName());
        if(fileInfo.isDir()){
            temp->setIcon(QIcon(":/icons/dir"));
        }else if(fileInfo.isFile()){
            temp->setIcon(QIcon(":/icons/file"));
        }else if(fileInfo.isSymLink()){
            temp->setIcon(QIcon(":/icons/link"));
        }
        ui->localFileInfo->addItem(temp);
    }

}

void FileExplorer::changeLocalWorkDir(QListWidgetItem* item){
    showLocalFileInfo(item->data(1).toString());
}
