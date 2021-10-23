#include "filecontainer.h"
#include<QDebug>
#include<QMimeData>
#include<QApplication>

FileContainer::FileContainer(QWidget* parent):
    QListWidget(parent)
{
    setAcceptDrops(true);
}

void FileContainer::mousePressEvent(QMouseEvent *event){
    //鼠标点击时调用
    //qDebug()<<"mouse press";
    if(event->button()==Qt::LeftButton){
        startPos=event->pos();
    }
    QListWidget::mousePressEvent(event);
}

void FileContainer::mouseMoveEvent(QMouseEvent *event){
    //鼠标拖动时调用，移动一定距离才视为需要进行拖动
    //qDebug()<<"mouse move"<<event->button()<<" "<<Qt::LeftButton;
    QListWidgetItem* item=itemAt(startPos);
    if(item==nullptr)return;
    if(item->data(5)!="file")return;//暂时不支持文件夹拖动
    int distance=(event->pos()-startPos).manhattanLength();
    //qDebug()<<"distance:"<<distance;
    if(distance>=QApplication::startDragDistance()){
        performDrag();
    }

}

void FileContainer::dragEnterEvent(QDragEnterEvent *event){
    // 拖拽目标进入范围时调用
    //qDebug()<<"drag enter";
    //LocalFileContainer* source=(LocalFileContainer*)((void*)event->source());
    QListWidget* source=qobject_cast<QListWidget*>(event->source());
    if(source&& source==this){
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else{
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void FileContainer::performDrag(){
    auto item=itemAt(startPos);
    QMimeData* mimeData=new QMimeData;
    //qDebug()<<"set mimedata:"<<item->data(0).toString();
    mimeData->setText(item->data(0).toString());
    QDrag *drag=new QDrag(this);
    drag->setMimeData(mimeData);
    if(drag->exec(Qt::CopyAction| Qt::MoveAction)==Qt::MoveAction){
        selectionModel()->clearSelection();
    }
}

void FileContainer::dragMoveEvent(QDragMoveEvent *event){
    QListWidget* source=qobject_cast<QListWidget*>(event->source());
    if(source&& source!=this){
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }else if(source==this){
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

