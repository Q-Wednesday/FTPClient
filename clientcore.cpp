#include "clientcore.h"

ClientCore::ClientCore(QString hostname,int port,QObject *parent) : QObject(parent),
    serverPort(port),connectionSocket(new QTcpSocket(this)),hostName(hostname),state(NOTCONNECTED),
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
    //TODO:接收完成后关闭数据连接
    //handleResponse(recv_text);
    return ;
}

void ClientCore::handleResponse(QString &response){
    if(response.startsWith("220")){
        state=CONNECTED;
        commandUSER();
    }
    else if(response.startsWith("331")){
        state=NOTLOGIN;
        commandPASS();
    }
    else if(response.startsWith("230")){
        state=LOGIN;
        commandSYST();
    }else if(response.startsWith("200")){
        //这是所有命令成功的标志，根据情况变更状态
        switch (state) {
            case REQTYPE:
                state=LOGIN;
                emit initSuccess(true);
            break;
            case REQPASV:
                state=PASVMODE;
            break;
            case REQPORT:
                state=PORTMODE;
            break;
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
        switch (state) {
        case REQLIST:
            state=LOGIN;
            qDebug()<<"start transfer";
        }
    }else if(response.startsWith("226")){
        qDebug()<<"transfer complete";
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
    state=REQTYPE;
    int n=connectionSocket->write(message.toLocal8Bit());
    qDebug()<<"send"<<message;

}

ClientCore::~ClientCore(){
    qDebug()<<"client destroyed";
}

ClientState ClientCore::getState(){
    return state;
}

void ClientCore::commandPORT(){
    //TODO: 随机化端口，正确的地址
    tcpServer=new QTcpServer(this);
    QHostAddress address("127.0.0.1");
    tcpServer->listen(QHostAddress::Any,1025);
    connect(tcpServer,&QTcpServer::newConnection,[this]{
        fileSocket=tcpServer->nextPendingConnection();
        connect(fileSocket,&QTcpSocket::readyRead,this,&ClientCore::receiveFile);
    });
    QString message=QString("PORT %1\r\n").arg("127,0,0,1,4,1");
    int n=connectionSocket->write(message.toLocal8Bit());
    qDebug()<<"send"<<message;
}

void ClientCore::commandLIST(QString path){
    //要已经进入模式才可以
    commandPORT();
    state=REQLIST;
    filePath=path;
}
