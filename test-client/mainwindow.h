#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../common/command.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void lamp(int a);

private:
    Ui::MainWindow *ui;

signals:
    void send(message);

private slots:
    void on_PBcert_clicked();
    void on_PBcovidC_clicked();
    void on_PBticket_clicked();
    void on_PBticketC_clicked();
    void on_pushButton_clicked();
};
#endif // MAINWINDOW_H
