#ifndef COVID_CERT_CHEKER_H
#define COVID_CERT_CHEKER_H

#include <QObject>
#include <QState>
#include <QSignalTransition>
#include <QTimer>
#include <wicketFSM/wicketfsm.h>

class CovidWrapper : public QState
{
    Q_OBJECT
public:

    QState *onReady;
    QState *onCheckBarcode;
    QState *onCheckCovid;
    QState *onCheckCovidContoller;
    QState *onShowCovidContollerFail;
    QState *onWaitCovidContollerCheck;  // проверка паспорта посетителя охраной
    QState *TicketHandler;
    QState *onShowCovidFail;
    QState *onShowNoTicket;            // show fail screen when ticket first

    CovidWrapper(QState *parent, QByteArray CovidContollerPrefix, QByteArray controllerPrefix, QByteArray covidQRPrefix);

private:
    QByteArray currentBarcode="";

    QByteArray CovidContollerPrefix;
    QByteArray ControllerPrefix;
    QByteArray covidQRPrefix;

    void set_timers();

    void addTransition();

    void setAction();

    void checkBarcode(QByteArray bc);



    QTimer *communicationTimer;
    QTimer *showTimer;
    QTimer *CovidContollerCheckTimer;
    QTimer *ticket_timer;

    int communicationTimeout = 5000;
    int showTimeout = 2000;
    int CovidContollerCheckTimeout = 30000;
    int ticket_timeout = 30000;

private slots:


signals:
    void barcode(QByteArray);
    void noTicket();
    void covidQR(QByteArray);  //
    void covidOK();  //
    void covidFail();  //
    void covidContollerQR();
    void covidContollerOK();
    void covidContollerFail();//
    void controllerQR();

    //outside
    void _ready();
    void _onShowNoTicket();
    void _onShowCovidFail();
    void _onShowCovidContollerFail();

};

#endif // COVID_CERT_CHEKER_H
