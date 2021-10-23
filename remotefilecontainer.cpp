#include "remotefilecontainer.h"
#include<QDebug>
#include<QMimeData>
#include<QMenu>
RemoteFileContainer::RemoteFileContainer(QWidget *parent) :
    FileContainer(parent)
{

}

void RemoteFileContainer::dropEvent(QDropEvent *event){
    auto data=event->mimeData();

    QListWidget* source=qobject_cast<QListWidget*>(event->source());
    if(source && source!=this){
        emit dragIn(data->text());
    }
}
void RemoteFileContainer::contextMenuEvent(QContextMenuEvent *event){
    QMenu *menu = new QMenu(this);
    auto item=itemAt(event->pos());
    if(item){
        if(item->data(5)=="file"){
            auto renameAction=new QAction(tr("&Rename"), menu);
            menu->addAction(renameAction);
            connect(renameAction,&QAction::triggered,this,[this,item]{
                emit renameFile(item->data(0).toString());
            });
        }
        else if(item->data(5)=="dir"){
            auto deleteAction=new QAction(tr("&Delete"), menu);
            menu->addAction(deleteAction);
            connect(deleteAction,&QAction::triggered,this,[this,item]{
                emit removeDir(item->data(0).toString());
            });
        }
    }else{
        auto makeDirAction=new QAction(tr("&New Directory"),menu);
        menu->addAction(makeDirAction);
        connect(makeDirAction,&QAction::triggered,this,[this]{
            emit makeDir();
        });
    }

    menu->move(cursor().pos()); //让菜单显示的位置在鼠标的坐标上
    menu->show();

}
