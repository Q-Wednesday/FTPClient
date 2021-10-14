#ifndef CLIENTCORE_H
#define CLIENTCORE_H

#include <QObject>
#include<QTcpSocket>
#include<QHostAddress>
#include<QHostInfo>
#include<QTcpServer>
#define MAX_MESSAGE_LEN 1024
#define MAX_DATA_SIZE 8192
enum ConnectionState{NOTCONNECTED,CONNECTED,NOTLOGIN,LOGIN,PORTMODE,PASVMODE,REQTYPE,REQPORT,REQPASV};
enum RequestState{NOTHING,REQLIST,REQCWD};
class ClientCore : public QObject
{
    Q_OBJECT
public:
    explicit ClientCore(QString hostname,int port=21,QObject *parent = nullptr);
    ~ClientCore();
    ConnectionState getState();
    bool connectServer();
    void commandUSER();
    void commandPASS();
    void commandSYST();
    void commandTYPE();
    void commandPORT();
    void commandPASV();
    void commandPWD();
    void commandCWD(QString dir);
    void commandLIST(QString path="");
    void userLogin(QString username,QString password="");//可以没有密码
signals:
    void initSuccess(bool);//所有初始连接步骤完成后才会发送，false为某一步失败
    void fileInfoGeted(QString);
    void pwdGeted(QString);
    void cwdSuccess(bool);
    void serverReponse(QString);
private:
    void handleResponse(QString& response);
private slots:
    void connectionSuccess();
    void receiveMessage();
    void receiveFile();
private:
    QTcpSocket* connectionSocket;
    QTcpSocket* fileSocket;
    QTcpServer* tcpServer;//用于port模式接受连接
    QHostAddress serverHost;
    QString hostName;//用户传入的名称
    int serverPort;
    bool connected;
    QString userName;
    QString passWord;
    QString filePath;
    QString workDir;
    ConnectionState connectionState;
    RequestState requestState;

};

#endif // CLIENTCORE_H
