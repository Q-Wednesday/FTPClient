#include "localfilecontainer.h"
#include<QDebug>
#include<QMimeData>
#include<QApplication>
LocalFileContainer::LocalFileContainer(QWidget *parent) :
    FileContainer(parent)
{


}


void LocalFileContainer::dropEvent(QDropEvent *event){
    auto data=event->mimeData();

    QListWidget* source=qobject_cast<QListWidget*>(event->source());
    if(source && source!=this){
        qDebug()<<"item data:"<<data->text();
        emit dragIn(data->text());//请求进行下载文件
    }
}


