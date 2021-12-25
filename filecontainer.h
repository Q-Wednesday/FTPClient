#ifndef FILECONTAINER_H
#define FILECONTAINER_H
#include<QListWidget>
#include<QDrag>
#include<QDragEnterEvent>
#include<QContextMenuEvent>
//本地文件管理器和远程文件管理器的基类，用于实现拖拽的功能。

class FileContainer:public QListWidget
{
    Q_OBJECT
public:
    explicit FileContainer(QWidget* parent = nullptr);
signals:
    void dragIn(QString);
    void renameFile(QString);
    void removeDir(QString);
    void makeDir();
protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void performDrag();

protected:
    QPoint startPos;
};

#endif // FILECONTAINER_H
