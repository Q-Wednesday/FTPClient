#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"clientcore.h"
#include"logindialog.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_loginButton_clicked();
    void newSession(ClientCore* client);
    void closeSession(int index);

private:
    Ui::MainWindow *ui;
    QTabWidget* sessionTab;
    LoginDialog* dialog;
};
#endif // MAINWINDOW_H
