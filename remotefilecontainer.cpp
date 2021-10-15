#include "remotefilecontainer.h"
#include<QDebug>
#include<QMimeData>
RemoteFileContainer::RemoteFileContainer(QWidget *parent) :
    FileContainer(parent)
{

}

void RemoteFileContainer::dropEvent(QDropEvent *event){
    auto data=event->mimeData();

    QListWidget* source=qobject_cast<QListWidget*>(event->source());
    if(source && source!=this){
        qDebug()<<"item data:"<<data->text();
        emit dragIn(data->text());
    }
}
