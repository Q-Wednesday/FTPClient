#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <QWidget>
#include<QListWidget>
#include<QListWidgetItem>
#include"clientcore.h"
#include"localfilecontainer.h"
#include"remotefilecontainer.h"
namespace Ui {
class FileExplorer;
}

class FileExplorer : public QWidget
{
    Q_OBJECT

public:
    explicit FileExplorer(QWidget *parent = nullptr);
    ~FileExplorer();
    void closeSession();
public slots:
    void bindClient(ClientCore* clientLogin);
private slots:
    void showRemoteFileInfo(QString infoReceived);
    void showRemoteWorkDir(QString workdir);
    void changeRemoteWorkDir(QListWidgetItem*);
    void showLocalFileInfo(QString localPath="");
    void changeLocalWorkDir(QListWidgetItem*);
    void openRemoteFile(QListWidgetItem*);
    void openLocalFile(QListWidgetItem*);
    void downloadSuccess();//成功下载
    void downloadFile(QString);//下载信号
    void uploadFile(QString);//上传文件
    void renameFile(QString);//重命名
    void removeDir(QString);//删除文件夹
    void makeDir();//创建文件夹
    void on_reconnectButton_clicked();

    void on_refreshButton_clicked();

signals:
    void sessionClosed();
private:
    Ui::FileExplorer *ui;
    ClientCore* client;

    QString localWorkDir;
    LocalFileContainer* localFileContainer;
    RemoteFileContainer* remoteFileContainer;
    bool doubleCliked;//判断点击事件 是不是double
};

#endif // FILEEXPLORER_H
