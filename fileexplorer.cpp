#include "fileexplorer.h"
#include "ui_fileexplorer.h"
#include<QIcon>
#include<QDir>
#include<QInputDialog>
#include<QTimer>
FileExplorer::FileExplorer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileExplorer),
    localFileContainer(new LocalFileContainer(this)),
    remoteFileContainer(new RemoteFileContainer(this)),
    doubleCliked(false)
{
    ui->setupUi(this);
    ui->fileListLayout->addWidget(remoteFileContainer);
    ui->fileListLayout->addWidget(localFileContainer);
    connect(ui->remoteDir,&QListWidget::itemClicked,this,&FileExplorer::changeRemoteWorkDir);
    showLocalFileInfo("D:");
   // connect(ui->localDir,&QListWidget::itemClicked,this,&FileExplorer::changeLocalWorkDir);
    connect(ui->localDir,&QListWidget::itemClicked,[this](QListWidgetItem* item){
        qDebug()<<"doubleclicked:"<<doubleCliked;
        if(!doubleCliked){
            QTimer::singleShot(100,this,[this,item]{changeLocalWorkDir(item);});

        }
    });
    connect(remoteFileContainer,&QListWidget::itemDoubleClicked,this,&FileExplorer::openRemoteFile);
    connect(localFileContainer,&QListWidget::itemDoubleClicked,this,&FileExplorer::openLocalFile);
    connect(localFileContainer,&LocalFileContainer::dragIn,this,&FileExplorer::downloadFile);
    connect(remoteFileContainer,&RemoteFileContainer::dragIn,this,&FileExplorer::uploadFile);
    connect(remoteFileContainer,&FileContainer::renameFile,this,&FileExplorer::renameFile);
    connect(remoteFileContainer,&FileContainer::removeDir,this,&FileExplorer::removeDir);
    connect(remoteFileContainer,&FileContainer::makeDir,this,&FileExplorer::makeDir);

    //双击可以输入路径
    connect(ui->localDir,&QListWidget::itemDoubleClicked,this,[this](QListWidgetItem* item){
        qDebug()<<"double click";
        doubleCliked=true;
        bool ok;
        QString target=QInputDialog::getText(this,tr("Change Directory"),tr("Enter Path"),QLineEdit::Normal,item->data(256).toString(),&ok);
        if(ok && !target.isEmpty()){
            showLocalFileInfo(target);
        }
    });
}

FileExplorer::~FileExplorer()
{
    delete ui;
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
    connect(client,&ClientCore::rntoSuccess,[this]{
        client->commandPWD();
    });
    connect(client,&ClientCore::rmdSuccess,[this]{
        client->commandPWD();
    });
    connect(client,&ClientCore::mkdSuccess,[this]{
        client->commandPWD();
    });
    connect(client,&ClientCore::quitSuccess,[this]{
        emit sessionClosed();
    });
    client->commandPWD();
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
    //先检测是否在
    QDir localDir=QDir(localPath);
    if(!localDir.exists())return;
    localWorkDir=localPath;//改变工作目录
    localFileContainer->clear();
    for(auto fileInfo:localDir.entryInfoList()){
        if(fileInfo.fileName()=="." || fileInfo.fileName()=="..")continue;
        qDebug()<<fileInfo.absoluteFilePath();
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
    //临时测试加的
    for(auto filename:localDir.entryList()){
        qDebug()<<filename;
    }

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


}

void FileExplorer::changeLocalWorkDir(QListWidgetItem* item){
    qDebug()<<"call change local"<<doubleCliked;
    if(!doubleCliked)
        showLocalFileInfo(item->data(256).toString());
    else doubleCliked=false;
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

void FileExplorer::renameFile(QString source){
    bool ok;
    QString target=QInputDialog::getText(this,tr("Rename File"),tr("Enter New File Name"),QLineEdit::Normal,source,&ok);
    if(ok && !target.isEmpty()){
        client->commandRNFR(source,target);
    }

}
void FileExplorer::removeDir(QString dir){
    client->commandRMD(dir);
}

void FileExplorer::makeDir(){
    bool ok;
    QString target=QInputDialog::getText(this,tr("New Directory"),tr("Enter Name"),QLineEdit::Normal,0,&ok);
    if(ok && !target.isEmpty()){
        client->commandMKD(target);
    }
}

void FileExplorer::closeSession(){
    client->commandQUIT();
}
