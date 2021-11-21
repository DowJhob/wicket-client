#include "w3.covid-cert-cheker.h"

checkCovidCert::checkCovidCert(_reader_type *reader_type,
                           dir_type *direction_type,
                           bool *ready_state_flag,
                           bool *uncond_state_flag,
                           QState *parent):QState(parent),
    //CovidContollerPrefix(CovidContollerPrefix), ControllerPrefix(controllerPrefix), covidQRPrefix(covidQRPrefix),
      ready_state_flag(ready_state_flag),
      uncond_state_flag(uncond_state_flag),
      reader_type(reader_type),
      direction_type(direction_type)
    //,
      //wicket(wicket)


{
    onReady                   = new QState( this );
    onCheckBarcode            = new QState( this );
    onCheckCovid              = new QState( this );
    onCheckCovidContoller     = new QState( this );
    onShowCovidContollerFail  = new QState( this );
    onWaitCovidContollerCheck = new QState( this );
    onShowCovidFail           = new QState( this );
    onShowNoTicket            = new QState( this );

    TicketHandler             = new wicketFSM(reader_type,
                                              direction_type,
                                              ready_state_flag,
                                              uncond_state_flag,
                                              this);

    connect(TicketHandler, &wicketFSM::showState, this, &checkCovidCert::showState);
    connect(TicketHandler, &wicketFSM::send_to_server, this, &checkCovidCert::send_to_server);

    set_timers();
    addTransitions();
    setAction();

    setInitialState( onReady );
}

void checkCovidCert::set_timers()
{
    communicationTimer  = new QTimer(this);
    showTimer  = new QTimer(this);
    CovidContollerCheckTimer = new QTimer(this);
    //ticket_timer     = new QTimer(this);

    communicationTimer->setSingleShot(true);
    showTimer->setSingleShot(true);
    CovidContollerCheckTimer->setSingleShot(true);
    //ticket_timer->setSingleShot(true);

    communicationTimer->setInterval(communicationTimeout);
    showTimer->setInterval(showTimeout);
    CovidContollerCheckTimer->setInterval(CovidContollerCheckTimeout);
    //ticket_timer->setInterval(ticket_timeout);
}

void checkCovidCert::addTransitions()
{
    onReady->addTransition(                   this,                     &checkCovidCert::barcode,            onCheckBarcode);
    onCheckBarcode->addTransition(            this,                     &checkCovidCert::covidQR,            onCheckCovid);
    onCheckBarcode->addTransition(            this,                     &checkCovidCert::noTicket,           onShowNoTicket);
    onShowNoTicket->addTransition(            showTimer,                &QTimer::timeout,                    onReady);
    onCheckCovid->addTransition(              this,                     &checkCovidCert::covidOK,            onWaitCovidContollerCheck);
    onWaitCovidContollerCheck->addTransition( this,                     &checkCovidCert::barcode,            onCheckBarcode);
    onWaitCovidContollerCheck->addTransition( CovidContollerCheckTimer, &QTimer::timeout,                    onReady);
    onCheckCovid->addTransition(              this,                     &checkCovidCert::covidFail,          onShowCovidFail);
    onCheckCovid->addTransition(              communicationTimer,       &QTimer::timeout,                    onReady);
    onShowCovidFail->addTransition(           showTimer,                &QTimer::timeout,                    onReady);
    onCheckBarcode->addTransition(            this,                     &checkCovidCert::covidContollerQR,   onCheckCovidContoller);
    onCheckCovidContoller->addTransition(     communicationTimer,       &QTimer::timeout,                    onReady);
    onCheckCovidContoller->addTransition(     this,                     &checkCovidCert::covidContollerFail, onShowCovidContollerFail);
    onCheckCovidContoller->addTransition(     this,                     &checkCovidCert::covidContollerOK,   TicketHandler);
    onCheckBarcode->addTransition(            this,                     &checkCovidCert::controllerQR,       TicketHandler);
    //TicketHandler->addTransition(             ticket_timer,             &QTimer::timeout,                    onReady);
    TicketHandler->addTransition(             TicketHandler,            &QState::finished,                   onReady);
}

void checkCovidCert::setAction()
{
    //connect(CheckBarcode, &QState::entered, &CovidWrapper::checkBarcode);
    connect(this, &checkCovidCert::barcode, &checkCovidCert::checkBarcode);

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

void checkCovidCert::checkBarcode(QByteArray bc)
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
