#ifndef WICKETFSM_H
#define WICKETFSM_H

#include <QObject>
#include <QState>
#include <QTimer>

class wicketFSM : public QState
{
    Q_OBJECT
public:
    //// Из любого состояния можно перейти в разблокированный турникет
    QState *Ready;
    QState *OnCheckEntry;
    QState *OnCheckEXit;
    QState *dbTimeout;
    QState *Entry;
    QState *entryPassed;
    QState *Exit;
    QState *exitPassed;
    QState *Wrong;
    QState *Drop;
    QState *UncondTimeout;


    wicketFSM(QState *parent):QState(parent) //// Из любого состояния можно перейти в разблокированный турникет
    {
        set_timer();
        Ready                 = new QState( this );
        OnCheckEntry          = new QState( this );
        OnCheckEXit           = new QState( this );
        dbTimeout             = new QState( this );
        Entry                 = new QState( this );
        entryPassed           = new QState( this );
        Exit                  = new QState( this );
        exitPassed            = new QState( this );
        Wrong                 = new QState( this );
        Drop                  = new QState( this );
        UncondTimeout         = new QState( this );

        setInitialState( Ready );
        //================================================================================================================================================================================
        Ready->addTransition(this, &wicketFSM::set_FSM_to_wrong, Wrong); // по сигналу с сервера переходим в состояния запрещено
        Ready->addTransition(  this,                   &wicketFSM::set_FSM_to_onCheckEntry, OnCheckEntry); //состояние на проверке
        Ready->addTransition(  this,                   &wicketFSM::set_FSM_to_onCheckEXit, OnCheckEXit); //состояние на проверке

        Wrong->addTransition(wrong_light_timer,        &QTimer::timeout, Ready); //по сигналу таймера тоже обратно
        dbTimeout->addTransition( Wrong);

        OnCheckEntry->addTransition(this,              &wicketFSM::set_FSM_to_wrong, Wrong); //wrong ticket
        OnCheckEntry->addTransition(this,              &wicketFSM::set_FSM_to_entry, Entry); // по сигналу с сервера переходим в состояние открыто на вход
        OnCheckEntry->addTransition(_db_Timeout_timer, &QTimer::timeout,             dbTimeout);

        OnCheckEXit->addTransition(this,               &wicketFSM::set_FSM_to_wrong, Wrong); //wrong ticket
        OnCheckEXit->addTransition(this,               &wicketFSM::set_FSM_to_exit,  Exit); // по сигналу с сервера переходим в состояние открыто на выход
        OnCheckEXit->addTransition(_db_Timeout_timer,  &QTimer::timeout,             dbTimeout);

        Entry->addTransition(wait_pass_timer,          &QTimer::timeout,             Drop);           //по сигналу таймера в состояние сброс прохода
        Entry->addTransition(this,                     &wicketFSM::set_FSM_passed,   entryPassed);            //по сигналу прохода от турникета перейдем в состояние проход
        entryPassed->addTransition( UncondTimeout );                                       // и сразу в задержку прохода по пути отослав данные на сервер

        Exit->addTransition(wait_pass_timer,           &QTimer::timeout,               Drop);
        Exit->addTransition(this,                      &wicketFSM::set_FSM_passed,     exitPassed);
        exitPassed->addTransition( Ready );

        UncondTimeout->addTransition(uncondDelayTimer, &QTimer::timeout,                   Ready);
        UncondTimeout->addTransition(this,             &wicketFSM::set_FSM_to_onCheckEXit, OnCheckEXit); //Для возможности быстро чекнуть билет на выход не дожидаясь таймаута на досмотр
        Drop->addTransition(Ready);

        //=================================================================================================
        connect(Ready,         &QState::entered, wrong_light_timer, &QTimer::stop );
        connect(Ready,         &QState::entered, wait_pass_timer,   &QTimer::stop );
        //=================================================================================================
        //=================================================================================================
        connect(OnCheckEntry,       SIGNAL(entered()), _db_Timeout_timer, SLOT(start()));
        connect(OnCheckEntry,       &QState::exited,   _db_Timeout_timer, &QTimer::stop );
        //=======                                                                   =======================
        connect(OnCheckEXit,       SIGNAL(entered()), _db_Timeout_timer, SLOT(start()));
        connect(OnCheckEXit,       &QState::exited,   _db_Timeout_timer, &QTimer::stop );
        //=================================================================================================
        //=================================================================================================
        connect(Wrong,         SIGNAL(entered()), wrong_light_timer, SLOT(start()) );
        connect(Wrong,       &QState::exited,   _db_Timeout_timer, &QTimer::stop );
        //=================================================================================================
        connect(Entry,         SIGNAL(entered()), wait_pass_timer, SLOT(start()) );  //вошли в состояния прохода, открыли калитку, зажгли лампы
        connect(Entry,       &QState::exited,  wait_pass_timer, &QTimer::stop );
        //==============                                              =====================================
        connect(Exit,          SIGNAL(entered()), wait_pass_timer, SLOT(start()) );
        connect(Exit,        &QState::exited,  wait_pass_timer, &QTimer::stop );
        //=================================================================================================
        //=================================================================================================
        connect(UncondTimeout, SIGNAL(entered()), uncondDelayTimer, SLOT(start()) );
        connect(UncondTimeout, &QState::exited,   uncondDelayTimer, &QTimer::stop );

    }
    void timer_wrapper()
    {

    }
public slots:
    void set_uncondDelay_time(int uncondDelay = 6000)
    {
        uncondDelayTimer->setInterval(uncondDelay);
    }

signals:
    void set_FSM_to_entry();
    void set_FSM_to_exit();
    void set_FSM_passed();
    void set_FSM_to_wrong();

    void set_FSM_to_onCheckEntry();
    void set_FSM_to_onCheckEXit();

private:
    void set_timer()
    {
        wait_pass_timer = new QTimer(this);
        wrong_light_timer = new QTimer(this);
        uncondDelayTimer = new QTimer(this);
        _db_Timeout_timer = new QTimer(this);

        _db_Timeout_timer->setSingleShot(true);
        wait_pass_timer->setSingleShot(true);
        wrong_light_timer->setSingleShot(true);
        uncondDelayTimer->setSingleShot(true);

        //uncondDelayTimer->setInterval(uncondDelay);
        set_uncondDelay_time();
        wait_pass_timer->setInterval(pass_wait_time);
        wrong_light_timer->setInterval(wrong_light_TIME);
        _db_Timeout_timer->setInterval(15000);
    }

    QTimer *wait_pass_timer;
    int pass_wait_time = 10000;
    QTimer *wrong_light_timer;
    int wrong_light_TIME = 2000;
    QTimer *_db_Timeout_timer;
    QTimer *uncondDelayTimer;
//    int uncondDelay = 8000;
};

#endif // WICKETFSM_H
