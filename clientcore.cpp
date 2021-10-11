#include "clientcore.h"

ClientCore::ClientCore(QString hostname,int port,QObject *parent) : QObject(parent),
    serverPort(port),connectionSocket(new QTcpSocket(this)),hostName(hostname)
{
    //构造函数只创建，不连接，不报错
    connect(connectionSocket,&QTcpSocket::connected,this,&ClientCore::connectionSuccess);
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
        if(connected)break;
    }

    return connected;
}

void ClientCore::connectionSuccess(){
    qDebug()<<"connection success";
}

bool ClientCore::commandUSER(QString username){
    if(connected){
        QString message=QString("USER %1\r\n").arg(username);
        int n=connectionSocket->write(message.toLocal8Bit());
        qDebug()<<n;
        if(n>0)return true;
    }
}
