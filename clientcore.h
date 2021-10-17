#ifndef CLIENTCORE_H
#define CLIENTCORE_H

#include <QObject>
#include<QTcpSocket>
#include<QHostAddress>
#include<QHostInfo>
#include<QTcpServer>
#include<QFile>
#define MAX_MESSAGE_LEN 1024
#define MAX_DATA_SIZE 8192
enum ConnectionState{NOTCONNECTED,CONNECTED,NOTLOGIN,LOGIN,PORTMODE,PASVMODE,REQTYPE,REQPORT,REQPASV};
enum RequestState{NOTHING,REQLIST,REQCWD,REQRETR,REQSTOR,REQRNFN,REQRNTO,REQRMD,REQMKD};
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
    void commandRETR(QString source,QString target);
    void commandSTOR(QString source,QString sourceDir);
    void commandRNFR(QString source,QString target);//一次存好
    void commandRNTO();
    void commandRMD(QString dir);
    void commandMKD(QString dir);
    void commandQUIT();
    QString getHostName();
signals:
    void initSuccess(bool);//所有初始连接步骤完成后才会发送，false为某一步失败
    void fileInfoGeted(QString);
    void pwdGeted(QString);
    void cwdSuccess(bool);
    void serverReponse(QString);
    void retrSuccess();
    void storSuccess();
    void rntoSuccess();
    void rmdSuccess();
    void mkdSuccess();
    void quitSuccess();
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
    QString filePath;//标示list的目标路径
    ConnectionState connectionState;
    RequestState requestState;
    QString sourceFile;//标识文件传输的 源文件
    QString targetDir;//标识文件传输的目标文件夹
    QString sourceDir;//标识文件传输的源文件夹
    QString targetName;//标识重命名的目标名称
    QFile* filePointer;
};

#endif // CLIENTCORE_H
