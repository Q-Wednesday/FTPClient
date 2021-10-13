#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <QWidget>
#include"clientcore.h"
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

private:
    Ui::FileExplorer *ui;
    ClientCore* client;
};

#endif // FILEEXPLORER_H
