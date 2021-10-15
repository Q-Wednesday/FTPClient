#ifndef LOCALFILECONTAINER_H
#define LOCALFILECONTAINER_H


#include"filecontainer.h"
class LocalFileContainer : public FileContainer
{
    Q_OBJECT

public:
    explicit LocalFileContainer(QWidget *parent = nullptr);

private:

    void dropEvent(QDropEvent *event);

};

#endif // LOCALFILECONTAINER_H
