#include "fileexplorer.h"
#include "ui_fileexplorer.h"
#include<QIcon>
#include<QDir>
FileExplorer::FileExplorer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileExplorer),
    localFileContainer(new LocalFileContainer(this)),
    remoteFileContainer(new RemoteFileContainer(this))
{
    ui->setupUi(this);
    ui->fileListLayout->addWidget(remoteFileContainer);
    ui->fileListLayout->addWidget(localFileContainer);
    connect(ui->remoteDir,&QListWidget::itemClicked,this,&FileExplorer::changeRemoteWorkDir);
    showLocalFileInfo();
    connect(ui->localDir,&QListWidget::itemClicked,this,&FileExplorer::changeLocalWorkDir);
    connect(remoteFileContainer,&QListWidget::itemDoubleClicked,this,&FileExplorer::openRemoteFile);
    connect(localFileContainer,&QListWidget::itemDoubleClicked,this,&FileExplorer::openLocalFile);
    connect(localFileContainer,&LocalFileContainer::dragIn,this,&FileExplorer::downloadFile);
    connect(remoteFileContainer,&RemoteFileContainer::dragIn,this,&FileExplorer::uploadFile);
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
    connect(client,&ClientCore::retrSuccess,this,&FileExplorer::downloadSuccess);
    connect(client,&ClientCore::storSuccess,[this]{
        client->commandPWD();
    });
}

void FileExplorer::showRemoteFileInfo(QString infoReceived){
    remoteFileContainer->clear();
    auto infos=infoReceived.split('\n');
    //实现解析文件信息
    //data 5用于标识文件类型 256存储文件真实的信息（全名等）
    for(auto info:infos){
        info=info.trimmed();
        if(info.isEmpty())continue;
        if(info[0]=='-'){
            QListWidgetItem* temp=new QListWidgetItem(info.split(' ').last());
            temp->setIcon(QIcon(":/icons/file"));
            temp->setData(5,"file");
            remoteFileContainer->addItem(temp);
        }else if(info[0]=='d'){
            QListWidgetItem* temp=new QListWidgetItem(info.split(' ').last());
            temp->setIcon(QIcon(":/icons/dir"));
            temp->setData(5,"dir");
            remoteFileContainer->addItem(temp);
        }else if(info[0]=='l'){
            QListWidgetItem* temp=new QListWidgetItem(info.split(' ').last());
            temp->setIcon(QIcon(":/icons/link"));
            temp->setData(5,"link");
            remoteFileContainer->addItem(temp);
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
    if(workdir=="/")dirs.pop_front();//分隔开的时候最前面一个不要.所有pwd是没有后缀/的
    for(auto dir:dirs){
        QListWidgetItem* temp=new QListWidgetItem(QString("%1/").arg(dir));
        prefix+=dir+'/';
        qDebug()<<"remote prefix"<<prefix;
        temp->setData(256,prefix);
        ui->remoteDir->addItem(temp);
    }
    client->commandLIST();

}

void FileExplorer::changeRemoteWorkDir(QListWidgetItem* item){
    client->commandCWD(item->data(256).toString());

}
void FileExplorer::showLocalFileInfo(QString localPath){

    if(localPath.isEmpty())
        localPath=QDir::currentPath();
    localWorkDir=localPath;//改变工作目录
    qDebug()<<"localPath:"<<localPath;
    auto dirs=localPath.split('/');
    QString prefix="";
    if(localPath=='/'){
        //mac或linux下
        dirs.pop_front();
    }
    ui->localDir->clear();
    for(auto dir:dirs){

        QListWidgetItem* temp=new QListWidgetItem(QString("%1/").arg(dir));
        qDebug()<<"dir:"<<dir;
        prefix+='/'+dir;
        qDebug()<<"prefix:"<<prefix<<" "<<prefix.mid(1);
        //windows下和mac,linux下有区别
        if(localPath[0]=='/')
            temp->setData(256,prefix);
        else
           temp->setData(256,prefix.mid(1));
        ui->localDir->addItem(temp);
    }

    QDir localDir=QDir(localPath);
    localFileContainer->clear();
    for(auto fileInfo:localDir.entryInfoList()){
        if(fileInfo.fileName()=="." || fileInfo.fileName()=="..")continue;
        QListWidgetItem* temp=new QListWidgetItem(fileInfo.fileName());
        if(fileInfo.isDir()){
            temp->setIcon(QIcon(":/icons/dir"));
            temp->setData(5,"dir");
        }else if(fileInfo.isFile()){
            temp->setIcon(QIcon(":/icons/file"));
            temp->setData(5,"file");
        }else if(fileInfo.isSymLink()){
            temp->setIcon(QIcon(":/icons/link"));
            temp->setData(5,"link");
        }
        localFileContainer->addItem(temp);
    }

}

void FileExplorer::changeLocalWorkDir(QListWidgetItem* item){
    showLocalFileInfo(item->data(256).toString());
}

void FileExplorer::openRemoteFile(QListWidgetItem* item){
    qDebug()<<"double click";
    //目前只处理打开文件夹,之后可能要支持打开软链接
    if(item->data(5).toString()=="dir"){
        client->commandCWD(item->data(0).toString());
    }

}
void FileExplorer::openLocalFile(QListWidgetItem* item){
    qDebug()<<"doubleclick";
    if(item->data(5).toString()=="dir"){
        showLocalFileInfo(localWorkDir+'/'+item->data(0).toString());
    }
}

void FileExplorer:: downloadFile(QString sourcefile){
    client->commandRETR(sourcefile,localWorkDir);
}

void FileExplorer::downloadSuccess(){
    showLocalFileInfo(localWorkDir);
}

void  FileExplorer::uploadFile(QString sourcefile){
    client->commandSTOR(sourcefile,localWorkDir);
}
