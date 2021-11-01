#include "covid-cert-cheker.h"

CovidWrapper::CovidWrapper(QState *parent, QByteArray CovidContollerPrefix, QByteArray controllerPrefix, QByteArray covidQRPrefix):QState(parent),
    CovidContollerPrefix(CovidContollerPrefix), ControllerPrefix(controllerPrefix), covidQRPrefix(covidQRPrefix)
{

    onReady                   = new QState( this );
    onCheckBarcode            = new QState( this );
    onCheckCovid              = new QState( this );
    onCheckCovidContoller     = new QState( this );
    onShowCovidContollerFail  = new QState( this );
    onWaitCovidContollerCheck = new QState( this );
    //      TicketHandler             = new wicketFSM(wicket, reader_type, direction_type, this);
    onShowCovidFail           = new QState( this );
    onShowNoTicket            = new QState( this );

    addTransition();
    setAction();

    setInitialState( onReady );
}

void CovidWrapper::set_timers()
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

void CovidWrapper::addTransition()
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

void CovidWrapper::setAction()
{
    //connect(CheckBarcode, &QState::entered, &CovidWrapper::checkBarcode);
    connect(this, &CovidWrapper::barcode, &CovidWrapper::checkBarcode);

    //       connect(new QEvent(), &CovidWrapper::barcode, &CovidWrapper::checkBarcode);

    connect(onReady,                  &QState::entered, [this](){emit _ready();});

    connect(onShowCovidFail,          &QState::entered, [this](){emit _onShowCovidFail();});
    connect(onShowNoTicket,           &QState::entered, [this](){emit _onShowNoTicket(); });
    connect(onShowCovidContollerFail, &QState::entered, [this](){emit _onShowCovidContollerFail();});

    connect(onCheckCovid, &QState::entered, [this]()
    {

    });
    connect(onReady, &QState::entered, [this](){emit _ready();});
    connect(onReady, &QState::entered, [this](){emit _ready();});

}

void CovidWrapper::checkBarcode(QByteArray bc)
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
