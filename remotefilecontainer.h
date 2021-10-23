#ifndef REMOTEFILECONTAINER_H
#define REMOTEFILECONTAINER_H

#include"filecontainer.h"

class RemoteFileContainer : public FileContainer
{
    Q_OBJECT

public:
    explicit RemoteFileContainer(QWidget *parent = nullptr);
protected:
    void contextMenuEvent(QContextMenuEvent*);
private:
    void dropEvent(QDropEvent*);
};

#endif // REMOTEFILECONTAINER_H
