#ifndef W3_H
#define W3_H

#include <QState>
#include <QObject>
#include "w3.covid-cert-cheker.h"

class counterCheck : public QState
{
    Q_OBJECT
public:
    checkCovidCert *Armed;

    counterCheck();

private:
    QState *onReady;
    QState *onComingTowards;

signals:
    void send_to_server(message);
    void showState(showStatus, QString);
};

#endif // W3_H
