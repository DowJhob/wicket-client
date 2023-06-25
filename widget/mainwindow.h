#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <wicket_widget.h>
#include <QHostAddress>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void logger(QString in);
    void readyRead(QString str);
    void parser(QString str, wicket_widget *wicket_widget_obj, QString readerDirectionType);

private slots:
    void tabArmed();
    void tabUnlock();

private:
    int i = 0;
    Ui::MainWindow *ui;
 QSpacerItem spacer;
 bool cross = false; //для чересполосицы в логе
 QString time_bg_style = "<span style=""background-color:#00CED1;"">";
 QString cross_bg_style_true = "<span style=""background-color:#B0E0E6;"">";
  QString cross_bg_style_false = "<span style=""background-color:#FFFFFF;"">";
int addTab( QString name );
wicket_widget *searchWidget(int tab, QString addr );


wicket_widget *search_wicket_widget_by_addr(QString addr);

private slots:
    void add_wicket(QTcpSocket *pClientSocket);
    void on_refreshButton_clicked();
    void lock_unlock(QString str)
    {
        emit send_to_socket(str);
    }
    void test_mode(QString str)
    {
        emit send_to_socket(str);
    }
    void on_normalButton_clicked();


    void on_unlockAll_pushButton_clicked();

    void on_armedAll_pushButton_clicked();

signals:
    void send_to_socket( const QString&);
    void reconnect_reset();
};

#endif // MAINWINDOW_H
