#include <QApplication>
#include <QObject>
#include <network.h>
#include <controller.h>
#include <mainwindow.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //a.setWindowIcon(QIcon("./icons/1-turniket.svg"));
    //controller _controller;
    network net;

    MainWindow m;

//    QObject::connect(&m, SIGNAL(reconnect_reset()), &net, SLOT(reconnect_reset()));
    QObject::connect(&net, SIGNAL(logger(QString)), &m, SLOT(logger(QString)));
    QObject::connect(&net, SIGNAL(readyRead(QString)), &m, SLOT(readyRead(QString)));
    QObject::connect(&m, SIGNAL(send_to_socket(QString)), &net, SLOT(send_to_socket(QString)));

 //   QList<Qt::GestureType> gestures;
 //   gestures << Qt::PanGesture << Qt::PinchGesture << Qt::SwipeGesture;
 //   m.setSizeConstraint(a);
 //   m.setFixedSize(1900, 1000);
    m.show();


    return a.exec();
}
