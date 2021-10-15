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

public slots:
    void bindClient(ClientCore* clientLogin);
private slots:
    void on_pushButton_clicked();
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
private:
    Ui::FileExplorer *ui;
    ClientCore* client;

    QString localWorkDir;
    LocalFileContainer* localFileContainer;
    RemoteFileContainer* remoteFileContainer;
};

#endif // FILEEXPLORER_H
