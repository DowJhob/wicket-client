#ifndef COVID_CERT_CHEKER_H
#define COVID_CERT_CHEKER_H

#include <QObject>
#include <QState>
#include <QSignalTransition>
#include <QTimer>
#include <wicketFSM/wicketfsm.h>

class checkCovidCert : public QState
{
    Q_OBJECT
public:
    wicketFSM *TicketHandler;

    checkCovidCert(//QByteArray CovidContollerPrefix, QByteArray controllerPrefix, QByteArray covidQRPrefix,
                 //nikiret *wicket,
                 _reader_type *reader_type,
                 dir_type *direction_type,
                 bool *ready_state_flag,
                 bool *uncond_state_flag,
                 QState *parent);

private:
    QState *onReady;
    QState *onCheckBarcode;
    QState *onCheckCovid;
    QState *onCheckCovidContoller;
    QState *onShowCovidContollerFail;
    QState *onWaitCovidContollerCheck;  // проверка паспорта посетителя охраной
    QState *onShowCovidFail;
    QState *onShowNoTicket;            // show fail screen when ticket first
    //QState *onCounterCheck;


    QByteArray currentBarcode="";

    QByteArray CovidContollerPrefix;
    QByteArray ControllerPrefix;
    QByteArray covidQRPrefix;

    bool *ready_state_flag;     //  поскольку нет простого способа узнать в каком состоянии машина
    bool *uncond_state_flag;    // сохраним пару состояний во флагах

    _reader_type *reader_type;
    dir_type *direction_type;

    nikiret *wicket;

    void set_timers();

    void addTransitions();

    void setAction();

    void checkBarcode(QByteArray bc);

    QTimer *communicationTimer;
    QTimer *showTimer;
    QTimer *CovidContollerCheckTimer;
    //QTimer *ticket_timer;

    int communicationTimeout = 5000;
    int showTimeout = 2000;
    int CovidContollerCheckTimeout = 30000;
    //int ticket_timeout = 30000;

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


    void send_to_server(message);
    void showState(showStatus, QString);

};

#endif // COVID_CERT_CHEKER_H
