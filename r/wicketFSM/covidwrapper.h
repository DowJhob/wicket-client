#ifndef COVIDWRAPPER_H
#define COVIDWRAPPER_H

#include <QObject>
#include <QState>
#include <QSignalTransition>
#include <QTimer>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QJsonDocument>




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

    CovidWrapper(QState *parent, QString CovidAPI, QByteArray CovidContollerPrefix, QByteArray controllerPrefix, QByteArray covidQRPrefix):QState(parent), CovidAPI(CovidAPI),
        CovidContollerPrefix(CovidContollerPrefix), ControllerPrefix(controllerPrefix), covidQRPrefix(covidQRPrefix)
    {

        mngr = new QNetworkAccessManager(this);
        connect(mngr, &QNetworkAccessManager::finished, this, &CovidWrapper::finish);

        onReady                   = new QState( this );
        onCheckBarcode            = new QState( this );
        onCheckCovid              = new QState( this );
        onCheckCovidContoller     = new QState( this );
        onShowCovidContollerFail  = new QState( this );
        onWaitCovidContollerCheck = new QState( this );
        TicketHandler             = new wicketFSM(this);
        onShowCovidFail           = new QState( this );
        onShowNoTicket            = new QState( this );

        addTransition();
        setConnect();

        setInitialState( onReady );
    }

private:
    QByteArray currentBarcode="";
    QByteArray current_unrzFull;

    QString CovidAPI ="https://www.gosuslugi.ru/api/covid-cert/v3/cert/check/";
    //https://www.gosuslugi.ru/api/covid-cert/v3/cert/check/9240000043378334?lang=ru&ck=cae135173610bdfc511c3522bfbefdae

    QByteArray CovidContollerPrefix;
    QByteArray ControllerPrefix;
    QByteArray covidQRPrefix;

    void set_timers()
    {
        communicationTimer  = new QTimer(this);
        showTimer  = new QTimer(this);
        CovidContollerCheckTimer = new QTimer(this);
        ticket_timer     = new QTimer(this);

        communicationTimer->setSingleShot(true);
        showTimer->setSingleShot(true);
        CovidContollerCheckTimer->setSingleShot(true);
        ticket_timer->setSingleShot(true);

        communicationTimer->setInterval(communicationTimeout);
        showTimer->setInterval(showTimeout);
        CovidContollerCheckTimer->setInterval(CovidContollerCheckTimeout);
        ticket_timer->setInterval(ticket_timeout);
    }

    void addTransition()
    {
        onReady->addTransition(                   this,                     &CovidWrapper::barcode,            onCheckBarcode);

        onCheckBarcode->addTransition(            this,                     &CovidWrapper::covidQR,            onCheckCovid);

        onCheckBarcode->addTransition(            this,                     &CovidWrapper::noTicket,           onShowNoTicket);

        onShowNoTicket->addTransition(            showTimer,                &QTimer::timeout,                  onReady);

        onCheckCovid->addTransition(              this,                     &CovidWrapper::covidOK,            onWaitCovidContollerCheck);

        onWaitCovidContollerCheck->addTransition( this,                     &CovidWrapper::barcode,            onCheckBarcode);

        onWaitCovidContollerCheck->addTransition( CovidContollerCheckTimer, &QTimer::timeout,                  onReady);

        onCheckCovid->addTransition(              this,                     &CovidWrapper::covidFail,          onShowCovidFail);

        onCheckCovid->addTransition(              communicationTimer,       &QTimer::timeout,                  onReady);

        onShowCovidFail->addTransition(           showTimer,                &QTimer::timeout,                  onReady);

        onCheckBarcode->addTransition(            this,                     &CovidWrapper::covidContollerQR,   onCheckCovidContoller);

        onCheckCovidContoller->addTransition(     communicationTimer,       &QTimer::timeout,                  onReady);

        onCheckCovidContoller->addTransition(     this,                     &CovidWrapper::covidContollerFail, onShowCovidContollerFail);

        onCheckCovidContoller->addTransition(     this,                     &CovidWrapper::covidContollerOK,   TicketHandler);





        onCheckBarcode->addTransition(            this,                     &CovidWrapper::controllerQR,       TicketHandler);

        TicketHandler->addTransition(             ticket_timer,             &QTimer::timeout,                  onReady);

        TicketHandler->addTransition(             TicketHandler,            &QState::finished,                 onReady);
    }

    void setConnect()
    {
        //connect(CheckBarcode, &QState::entered, &CovidWrapper::checkBarcode);
        connect(this, &CovidWrapper::barcode, &CovidWrapper::checkBarcode);

        connect(onReady,                  &QState::entered, [this](){emit _ready();});

        connect(onShowCovidFail,          &QState::entered, [this](){emit _onShowCovidFail();});
        connect(onShowNoTicket,           &QState::entered, [this](){emit _onShowNoTicket(); });
        connect(onShowCovidContollerFail, &QState::entered, [this](){emit _onShowCovidContollerFail();});

        connect(onCheckCovid, &QState::entered, [this]()
        {
            QString id;

            QUrl CovidAPIURL = QUrl(CovidAPI + id);

            QUrlQuery query;
            query.addQueryItem("lang", "ru");
            query.addQueryItem("ck", "cae135173610bdfc511c3522bfbefdae");
            CovidAPIURL.setQuery(query);
            mngr->get(QNetworkRequest(CovidAPIURL));
        });
        connect(onReady, &QState::entered, [this](){emit _ready();});
        connect(onReady, &QState::entered, [this](){emit _ready();});

    }

    void checkBarcode(QByteArray bc)
    {
        currentBarcode = bc;

        if(bc.contains(covidQRPrefix))
            emit covidQR(bc);
        else
        {
            QList<QByteArray> var = bc.split('/');
            if(var.size() < 2)
                emit noTicket();
            else if(var[0] == CovidContollerPrefix)
                emit covidContollerQR();
            if(var[0] == ControllerPrefix)
                emit controllerQR();
            else
                emit noTicket();
        }
    }

    void checkCovidCert(QByteArray bc)
    {

    }

    void getCertParamFromQuery()
    {

    }
    QNetworkAccessManager *mngr;

    QTimer *communicationTimer;
    QTimer *showTimer;
    QTimer *CovidContollerCheckTimer;
    QTimer *ticket_timer;

    int communicationTimeout = 5000;
    int showTimeout = 2000;
    int CovidContollerCheckTimeout = 30000;
    int ticket_timeout = 30000;

private slots:
    void finish(QNetworkReply *reply)
    {
        QJsonDocument Json = QJsonDocument::fromJson(reply->readAll());
        //emit done();
    }

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

#endif // COVIDWRAPPER_H
