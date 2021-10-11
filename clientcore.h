#ifndef CLIENTCORE_H
#define CLIENTCORE_H

#include <QObject>
#include<QTcpSocket>
#include<QHostAddress>
#include<QHostInfo>
class ClientCore : public QObject
{
    Q_OBJECT
public:
    explicit ClientCore(QString hostname,int port=21,QObject *parent = nullptr);
    bool connectServer();
    bool commandUSER(QString username);
signals:
private:

private slots:
    void connectionSuccess();
private:
    QTcpSocket* connectionSocket;
    QHostAddress serverHost;
    QString hostName;//用户传入的名称
    int serverPort;
    bool connected;
    QString userName;
};

#endif // CLIENTCORE_H
