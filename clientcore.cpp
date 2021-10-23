#include "clientcore.h"
#include<QFile>
#include<QNetworkInterface>
ClientCore::ClientCore(QString hostname,int port,QObject *parent) : QObject(parent),
    serverPort(port),connectionSocket(new QTcpSocket(this)),hostName(hostname),connectionState(NOTCONNECTED),
    fileSocket(new QTcpSocket(this)),passive(true)
{
    //构造函数只创建并获取本机IP，不进行连接操作

    connect(connectionSocket,&QTcpSocket::readyRead,this,&ClientCore::receiveMessage);//接收消息并读取
    connect(connectionSocket,&QTcpSocket::disconnected,this,&ClientCore::remoteClosed);//远端关闭就发送
    connect(fileSocket,&QTcpSocket::readyRead,this,&ClientCore::receiveFile);//接收文件传输

    //获取本机IP
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
       // 使用第一个不是Localhost的IPV4地址
       for (int i = 0; i < ipAddressesList.size(); ++i) {
           if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
               ipAddressesList.at(i).toIPv4Address()) {
               clientAddress = ipAddressesList.at(i);
               break;
           }
       }
       // 如果找不到就使用Localhost
       if (clientAddress.toString().isEmpty())
           clientAddress = QHostAddress(QHostAddress::LocalHost);

}

bool ClientCore::connectServer(){
    //由文件浏览器和登录界面等调用，进行服务器的连接步骤
    QHostInfo info=QHostInfo::fromName(hostName);
    connectionState=NOTCONNECTED;

    //连接直到连接上一个地址为止
    for(auto address:info.addresses()){
        if(address.protocol()!=QAbstractSocket::IPv4Protocol)continue;
        serverHost=address;
        connectionSocket->connectToHost(serverHost,serverPort);
        connected=connectionSocket->waitForConnected();
        if(connected){
            connectionState=CONNECTED;
            break;
        }
    }

    if(connected==false)
        emit initSuccess(false);//连接失败直接告知
    return connected;
}

void ClientCore::sendMessage(QString &message){
    //发送消息的函数
    int n=connectionSocket->write(message.toLocal8Bit());
    //qDebug()<<"send"<<message;
    if(n==-1) emit remoteClosed();//如果发送失败直接告知已经失去连接

}

void ClientCore::receiveMessage(){
    //接收消息的函数，如果没有可读的消息直接返回
    if(connectionSocket->bytesAvailable()<=0)
        return;

    QString recv_text=QString::fromLocal8Bit(connectionSocket->readAll());
    //支持处理同时到达的多行消息
    auto textList=recv_text.split("\r\n");
    QStringList tempList;
    //考虑到有多行信息，遇到三个数字一个空格的为截止
    for(auto text:textList){
        if(text.isEmpty())continue;
        if(text[3]==' '){
            tempList.append(text);
            QString message=tempList.join("/r/n");
            tempList.clear();
            //qDebug()<<text<<" "<<message;
            handleResponse(message);

        }else{
            tempList.append(text);
        }
    }

}
void ClientCore::receiveFile(){
    //接收文件传输
    if(fileSocket->bytesAvailable()<0)
        return;

    if(requestState==REQLIST){
        QString recv_text=QString::fromLocal8Bit(fileSocket->readAll());
        //qDebug()<<"received text:"<<recv_text;
        requestState=NOTHING;
        fileSocket->close();
        emit fileInfoGeted(recv_text);
    }
    else if(requestState==REQRETR){ //由于接收的文件会较大，要做不同的处理
        //qDebug()<<"receive file signal";
        while (true) {
            QByteArray data=fileSocket->read(MAX_DATA_SIZE);
            int m=filePointer->write(data);
            //qDebug()<<"write"<<m;
            if(data.size()==0)break;           
        }
    }
}

void ClientCore::handleResponse(QString &response){
    //处理服务器response的核心函数，正确和错误的返回码都有对应的处理逻辑。
    emit serverReponse(response);//将收到的response告知文件管理器
    if(response.startsWith("220")){
        connectionState=CONNECTED;
        commandUSER();//连接成功后自动进行USER请求
    }
    else if(response.startsWith("331")){
        connectionState=NOTLOGIN;
        commandPASS();//331说明需要登录，发出PASS
    }
    else if(response.startsWith("230")){
        connectionState=LOGIN; //此时已经登录成功了
        emit initSuccess(true);//发送消息并请求SYST
        commandSYST();
    }
    else if(response.startsWith("215")){
        commandTYPE();//SYST请求成功后发送TYPE请求
    }
    else if(response.startsWith("200")){
        //PORT和TYPE的成功标志
        if(requestState==REQTYPE){
            requestState=NOTHING;
        }
        if(connectionState==REQPORT){
            handleFileCommand();
        }
    }
    else if(response.startsWith("150")){
        //数据通道建立成功的标志，RETR和LIST由receiveMessage自动处理了
        //STOR需要发送
        switch (requestState) {
            case REQRETR:
            case REQLIST:
                //qDebug()<<"start transfer";
                break;
            case REQSTOR:{
                QFile file(sourceDir+'/'+sourceFile);
                //qDebug()<<"file info:"<<file.fileName()<<file.exists()<<file.size();
                file.open(QIODevice::ReadOnly);
                char buf[MAX_DATA_SIZE];
                while(true){
                    int n=file.read(buf,MAX_DATA_SIZE);
                    if(n==0)break;
                    int m=fileSocket->write(buf,n);
                    //qDebug()<<"stor m:"<<m<<" n:"<<n;
                }
                fileSocket->close();
                //qDebug()<<"stor file";
                break;
            }
        }
    }else if(response.startsWith("226")){
        //传输结束
        if(requestState==REQSTOR){
            requestState=NOTHING;
            emit storSuccess();            
        }

    }else if(response.startsWith("227")){
        //PASV连接消息，解析其中的6个数字
        if(connectionState==REQPASV){
            int pos1=response.indexOf('(');
            int pos2=response.indexOf(')');
            QString address=response.mid(pos1+1,pos2-pos1-1);
            auto list=address.split(',');
            int port=list[4].toInt()*256+list[5].toInt();
            address=list.mid(0,4).join('.');
            //qDebug()<<address<<port;
            fileSocket->connectToHost(QHostAddress(address),port);
            int c=fileSocket->waitForConnected();
            //qDebug()<<"filesocket connected:"<<c;
            if(c){
                connectionState=PASVMODE;
                handleFileCommand();
            }
        }

    }else if(response.startsWith("257")){
        if(requestState==REQPWD){
            //pwd命令，解析出路径地址并发信号
            QRegExp regExp(QString("\"(.*)\""));
            regExp.indexIn(response);
            requestState=NOTHING;
            emit pwdGeted(regExp.cap(1));
        }
        else if(requestState==REQMKD){
            requestState=NOTHING;
            emit rmdSuccess();
        }

    }
    else if(response.startsWith("250")){
        switch (requestState) {
            case REQCWD:{
                requestState=NOTHING;
                emit cwdSuccess(true);
                qDebug()<<"cwd success";
                commandPWD();//进行一次拉取，会成功重新拉取文件
                break;
            }
            case REQRNTO:{
                requestState=NOTHING;
                emit rntoSuccess();
                break;
            }
            case REQRMD:{
                requestState=NOTHING;
                emit rmdSuccess();
                break;
            }
            case REQMKD:{
            //某些server也会以250返回mkd结果
                requestState=NOTHING;
                emit mkdSuccess();
                break;
            }
        }

    }else if(response.startsWith("350")){
        //重命名成功接收命令
        commandRNTO();
    }else if(response.startsWith("221")){
        //good bye
        emit quitSuccess();
    }
    else if(response.startsWith("4")|| response.startsWith("5")){
        if(connectionState<LOGIN){
            //还没登录，告知初始化失败
            emit initSuccess(false);
        }else{
            //错误时把状态置为最普通的连接状态
            //如果服务器已经断开连接，在下一次client发送消息时可以感知到
            connectionState=LOGIN;
            requestState=NOTHING;
            emit commandFailed("");
        }
    }
}

void ClientCore::handleFileCommand(){
    //专门应对文件传输的三种情况，PORT和PASV模式下会在不同的时候调用
    //PASV在连接上主机后马上调用，而PORT在收到200后调用
    QString message;
    switch (requestState)  {
        case REQLIST:
        {
            if(filePath.length())
                message=QString("LIST %1\r\n").arg(filePath);
            else message=QString("LIST\r\n");
            sendMessage(message);
            break;
        }
        case REQRETR:{
            message=QString("RETR %1\r\n").arg(sourceFile);
            filePointer=new QFile(targetDir+"/"+sourceFile);
            if(filePointer->open(QIODevice::WriteOnly)==false){
                emit commandFailed("Can't open local file");
                break;
            }
            connect(fileSocket,&QTcpSocket::disconnected,[this]{
                filePointer->close();
                requestState=NOTHING;
                delete filePointer;
                emit retrSuccess();
            });
            sendMessage(message);
            break;
        }
        case REQSTOR:{
            message=QString("STOR %1\r\n").arg(sourceFile);
            sendMessage(message);
            break;
        }
    }
}
void ClientCore::commandUSER(){
    if(connected){
        QString message=QString("USER %1\r\n").arg(userName);
        sendMessage(message);
    }
}
void ClientCore::commandPASS(){
        QString message=QString("PASS %1\r\n").arg(passWord);
        sendMessage(message);

}
void  ClientCore::userLogin(QString username, QString password){
    userName=username;
    passWord=password;
}

void ClientCore::commandSYST(){
    QString message=QString("SYST\r\n");
    sendMessage(message);
}
void ClientCore::commandTYPE(){
    QString message=QString("TYPE I\r\n");
    requestState=REQTYPE;
    sendMessage(message);
}

ClientCore::~ClientCore(){
    qDebug()<<"client destroyed";
}

ConnectionState ClientCore::getState(){
    return connectionState;
}

void ClientCore::commandPORT(){

    tcpServer=new QTcpServer(this);
    connectionState=REQPORT;
    int random_port=20000+qrand()%45535;
    tcpServer->listen(QHostAddress::Any,random_port);
    connect(tcpServer,&QTcpServer::newConnection,[this]{
        fileSocket=tcpServer->nextPendingConnection();
        connect(fileSocket,&QTcpSocket::readyRead,this,&ClientCore::receiveFile);//接收文件传输
        if(requestState==REQRETR){
            connect(fileSocket,&QTcpSocket::disconnected,[this]{
                filePointer->close();
                delete filePointer;
                emit retrSuccess();
            });
        }

        connectionState=PORTMODE;
    });
    QString address=clientAddress.toString().replace(".",",");

    QString message=QString("PORT %1,%2,%3\r\n").arg(address).arg(random_port/256)
            .arg(random_port%256);
    sendMessage(message);
}
void ClientCore::commandPASV(){
    QString message=QString("PASV\r\n");
    connectionState=REQPASV;
    sendMessage(message);
}
void ClientCore::commandLIST(QString path){
    //要已经进入模式才可以
    //commandPORT();
    requestState=REQLIST;
    filePath=path;
    if(passive)
        commandPASV();
    else
        commandPORT();
}

void ClientCore::commandPWD(){
    requestState=REQPWD;
    QString message=QString("PWD\r\n");
    sendMessage(message);
}

void ClientCore::commandCWD(QString dir){
    requestState=REQCWD;
    QString message=QString("CWD %1\r\n").arg(dir);
    sendMessage(message);
}

void ClientCore::commandRETR(QString source,QString target){
    sourceFile=source;
    targetDir=target;
    requestState=REQRETR;
    if(passive)
        commandPASV();
    else
        commandPORT();
}
void ClientCore::commandSTOR(QString source,QString sourceDir){
    //source应该包含路径名
    sourceFile=source;
    this->sourceDir=sourceDir;
    requestState=REQSTOR;
    if(passive)
        commandPASV();
    else
        commandPORT();
}
void ClientCore::commandRNFR(QString source, QString target){
    sourceFile=source;
    targetName=target;
    requestState=REQRNFN;
    QString message=QString("RNFR %1\r\n").arg(sourceFile);
    sendMessage(message);
}

void ClientCore::commandRNTO(){
    requestState=REQRNTO;
    QString message=QString("RNTO %1\r\n").arg(targetName);
    sendMessage(message);

}
void ClientCore::commandRMD(QString dir){
    requestState=REQRMD;
    QString message=QString("RMD %1\r\n").arg(dir);
    sendMessage(message);
}

void ClientCore::commandMKD(QString dir){
    requestState=REQMKD;
    QString message=QString("MKD %1\r\n").arg(dir);
    sendMessage(message);
}
void ClientCore::commandQUIT(){
    QString message=QString("QUIT\r\n");
    sendMessage(message);
}

QString ClientCore::getHostName(){
    return hostName;
}

void ClientCore::setPassive(bool isPassive){
    passive=isPassive;
}
