#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <network.h>

//#include <wicket_object.h>
#include <wicket_widget.h>

#include <QDebug>

class controller: public QObject
{
    Q_OBJECT
public:
network net;
    controller()
    {

    }
private:


public slots:
    void new_wicket()
{

}
signals:
    void logger( QString );
};

#endif // CONTROLLER_H
