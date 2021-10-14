#include "clientcore.h"

ClientCore::ClientCore(QString hostname,int port,QObject *parent) : QObject(parent),
    serverPort(port),connectionSocket(new QTcpSocket(this)),hostName(hostname),connectionState(NOTCONNECTED),
    fileSocket(nullptr)
{
    //构造函数只创建，不连接，不报错
    connect(connectionSocket,&QTcpSocket::connected,this,&ClientCore::connectionSuccess);
    connect(connectionSocket,&QTcpSocket::readyRead,this,&ClientCore::receiveMessage);
}

bool ClientCore::connectServer(){
    QHostInfo info=QHostInfo::fromName(hostName);
    info.addresses().first();
    //连接直到连接上一个地址为止
    for(auto address:info.addresses()){
        if(address.protocol()!=QAbstractSocket::IPv4Protocol)continue;
        serverHost=address;
        connectionSocket->connectToHost(serverHost,serverPort);
        connected=connectionSocket->waitForConnected();
    }
    return connected;
}
void ClientCore::receiveMessage(){
    if(connectionSocket->bytesAvailable()<=0)
        return;
    //注意收发两端文本要使用对应的编解码
    QString recv_text=QString::fromLocal8Bit(connectionSocket->readAll());
    qDebug()<<"received"<<recv_text;
    handleResponse(recv_text);
    return ;
}
void ClientCore::receiveFile(){
    if(fileSocket->bytesAvailable()<=0)
        return;
    //注意收发两端文本要使用对应的编解码
    QString recv_text=QString::fromLocal8Bit(fileSocket->readAll());
    qDebug()<<"received file"<<recv_text;
    if(requestState==REQLIST){
        requestState=NOTHING;
        emit fileInfoGeted(recv_text);
    }
    fileSocket->close();
    delete fileSocket;
    //TODO:接收完成后关闭数据连接
    //handleResponse(recv_text);
    return ;
}

void ClientCore::handleResponse(QString &response){
    if(response.startsWith("220")){
        connectionState=CONNECTED;
        commandUSER();
    }
    else if(response.startsWith("331")){
        connectionState=NOTLOGIN;
        commandPASS();
    }
    else if(response.startsWith("230")){
        connectionState=LOGIN;
        commandSYST();
    }else if(response.startsWith("200")){
        //这是所有命令成功的标志，根据情况变更状态
        switch (connectionState) {
            case REQTYPE:
                connectionState=LOGIN;
                emit initSuccess(true);
            break;
            case REQPASV:
                connectionState=PASVMODE;
            break;
            case REQPORT:
                connectionState=PORTMODE;
            break;
        }
        switch (requestState) {
        case REQLIST:
        QString message;
        if(filePath.length())
            message=QString("LIST %1\r\n").arg(filePath);
        else message=QString("LIST\r\n");
        int n=connectionSocket->write(message.toLocal8Bit());
        qDebug()<<"send"<<message;
        break;
        }
    }
    else if(response.startsWith("215")){
        commandTYPE();
    }else if(response.startsWith("150")){
        switch (requestState) {
        case REQLIST:
            //connectionState=LOGIN;
            qDebug()<<"start transfer";
        }
    }else if(response.startsWith("226")){
        qDebug()<<"transfer complete";
    }else if(response.startsWith("227")){
        //PASV连接消息，解析其中的6个数字
        if(connectionState==REQPASV){
            int pos1=response.indexOf('(');
            int pos2=response.indexOf(')');
            QString address=response.mid(pos1+1,pos2-pos1-1);
            //address.replace(',','.');
            auto list=address.split(',');
            int port=list[4].toInt()*256+list[5].toInt();
            address=list.mid(0,4).join('.');
            fileSocket=new QTcpSocket(this);
            qDebug()<<address<<port;
            fileSocket->connectToHost(QHostAddress(address),port);
            connect(fileSocket,&QTcpSocket::readyRead,this,&ClientCore::receiveFile);
            int c=connectionSocket->waitForConnected();
            if(c){
                connectionState=PASVMODE;
                //TODO:这一段是重复代码，需要重新修改
                switch (requestState) {
                case REQLIST:
                QString message;
                if(filePath.length())
                    message=QString("LIST %1\r\n").arg(filePath);
                else message=QString("LIST\r\n");
                int n=connectionSocket->write(message.toLocal8Bit());
                qDebug()<<"send"<<message;
                break;
                }
            }

        }

    }else if(response.startsWith("257")){
        //pwd命令
        QRegExp regExp(QString("\"(.*)\""));
        regExp.indexIn(response);
        qDebug()<<regExp.cap(1);
        emit pwdGeted(regExp.cap(1));
    }
    else if(response.startsWith("250")){
        if(requestState==REQCWD){
            requestState=NOTHING;
            emit cwdSuccess(true);
            commandPWD();//进行一次拉去，会成功重新拉取文件
        }
    }
    else{
        emit initSuccess(false);
    }
}
void ClientCore::connectionSuccess(){
    qDebug()<<"connection success";
}

void ClientCore::commandUSER(){
    if(connected){
        QString message=QString("USER %1\r\n").arg(userName);
        int n=connectionSocket->write(message.toLocal8Bit());
        qDebug()<<"send"<<message;
    }
}
void ClientCore::commandPASS(){
        QString message=QString("PASS %1\r\n").arg(passWord);
        int n=connectionSocket->write(message.toLocal8Bit());
        qDebug()<<"send"<<message;

}
void  ClientCore::userLogin(QString username, QString password){
    userName=username;
    passWord=password;
}

void ClientCore::commandSYST(){
    QString message=QString("SYST\r\n");
    int n=connectionSocket->write(message.toLocal8Bit());
    qDebug()<<"send"<<message;
}
void ClientCore::commandTYPE(){
    QString message=QString("TYPE I\r\n");
    connectionState=REQTYPE;
    int n=connectionSocket->write(message.toLocal8Bit());
    qDebug()<<"send"<<message;

}

ClientCore::~ClientCore(){
    qDebug()<<"client destroyed";
}

ConnectionState ClientCore::getState(){
    return connectionState;
}

void ClientCore::commandPORT(){
    //TODO: 随机化端口，正确的地址.由于防火墙只有本机能实现
    tcpServer=new QTcpServer(this);
    QHostAddress address("127.0.0.1");
    tcpServer->listen(QHostAddress::Any,1025);
    connect(tcpServer,&QTcpServer::newConnection,[this]{
        fileSocket=tcpServer->nextPendingConnection();
        connect(fileSocket,&QTcpSocket::readyRead,this,&ClientCore::receiveFile);
    });
    QString localHostname=QHostInfo::localHostName();
    QHostInfo hostInfo=QHostInfo::fromName(localHostname);

    QString message=QString("PORT %1\r\n").arg("127,0,0,1,4,1");
    int n=connectionSocket->write(message.toLocal8Bit());
    qDebug()<<"send"<<message;
}
void ClientCore::commandPASV(){
    QString message=QString("PASV\r\n");
    connectionState=REQPASV;
    int n=connectionSocket->write(message.toLocal8Bit());
    qDebug()<<"send"<<message;
}
void ClientCore::commandLIST(QString path){
    //要已经进入模式才可以
    //commandPORT();
    commandPASV();
    requestState=REQLIST;
    filePath=path;
}

void ClientCore::commandPWD(){
    QString message=QString("PWD\r\n");
    int n=connectionSocket->write(message.toLocal8Bit());
    qDebug()<<"send"<<message;
}

void ClientCore::commandCWD(QString dir){
    //workDir=dir;
    QString message=QString("CWD %1\r\n").arg(dir);
    int n=connectionSocket->write(message.toLocal8Bit());
    requestState=REQCWD;
    qDebug()<<"send"<<message;
}
