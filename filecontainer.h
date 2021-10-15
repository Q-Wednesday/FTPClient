#ifndef FILECONTAINER_H
#define FILECONTAINER_H
#include<QListWidget>
#include<QDrag>
#include<QDragEnterEvent>
//本地文件管理器和远程文件管理器的基类，用于实现拖拽的功能。

class FileContainer:public QListWidget
{
    Q_OBJECT
public:
    explicit FileContainer(QWidget* parent = nullptr);
signals:
    void dragIn(QString);
protected:
    void dragEnterEvent(QDragEnterEvent*);
    //void dropEvent(QDropEvent *event);//个性化的行为，不重写
    void dragMoveEvent(QDragMoveEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void performDrag();
protected:
    QPoint startPos;
};

#endif // FILECONTAINER_H
