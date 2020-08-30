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
    QState *OnCheck;
    QState *dbTimeout;
    QState *Entry;
    QState *entryPassed;
    QState *Exit;
    QState *exitPassed;
    QState *Wrong;
    QState *Drop;
    QState *UncondTimeout;


    wicketFSM(QState *parent):QState(parent)
    {
        set_timer();
        //// Из любого состояния можно перейти в разблокированный турникет
        Ready                 = new QState( this );
        OnCheck               = new QState( this );
        dbTimeout            = new QState( this );
        Entry                 = new QState( this );
        entryPassed             = new QState( this );
        Exit                  = new QState( this );
        exitPassed              = new QState( this );
        Wrong                 = new QState( this );
        Drop                  = new QState( this );
        UncondTimeout         = new QState( this );

        setInitialState( Ready );
        //================================================================================================================================================================================
        Ready->addTransition(this, &wicketFSM::set_FSM_to_wrong, Wrong);              // по сигналу с сервера переходим в состояния запрещено
//        _wicketWrong->addTransition(this, &wicketFSM::from_server_to_ready, _wicketReady);              //и обратно
        Wrong->addTransition(wrong_light_timer, &QTimer::timeout, Ready);        //по сигналу таймера тоже обратно

        ///------------------------------------- собственно тут проходы ----------------------------------
        Ready->addTransition(  this,                     &wicketFSM::set_FSM_to_onCheck, OnCheck);             //состояние на проверке
        OnCheck->addTransition(this,                     &wicketFSM::set_FSM_to_wrong,   Wrong);             //wrong ticket

        OnCheck->addTransition(_db_Timeout_timer,        &QTimer::timeout,               dbTimeout);
        dbTimeout->addTransition( Wrong);

        OnCheck->addTransition(this,                     &wicketFSM::set_FSM_to_entry,   Entry);            // по сигналу с сервера переходим в состояние открыто
        Entry->addTransition(wait_pass_timer,            &QTimer::timeout,               Drop);           //по сигналу таймера в состояние сброс прохода
        Entry->addTransition(this,                       &wicketFSM::set_FSM_passed,     entryPassed);            //по сигналу прохода от турникета перейдем в состояние проход
        entryPassed->addTransition( UncondTimeout );                                       // и сразу в задержку прохода по пути отослав данные на сервер

        OnCheck->addTransition(this,                     &wicketFSM::set_FSM_to_exit,    Exit);

//Ready->addTransition(this,                     &wicketFSM::set_FSM_to_exit,    Exit); //В случае когда мы определены как двойной ридер нужно разрешить прямые переходы от связанного ридера
//Entry->addTransition(this,                     &wicketFSM::set_FSM_to_,    Ready);//В случае когда мы определены как выходной, по команде с сервера потушить лампы

        Exit->addTransition(wait_pass_timer,             &QTimer::timeout,               Drop);
        Exit->addTransition(this,                        &wicketFSM::set_FSM_passed,     exitPassed);
        //exitPassed->addTransition( UncondTimeout );
        exitPassed->addTransition( Ready );

        UncondTimeout->addTransition(UncondTimeoutTimer, &QTimer::timeout,               Ready);
        UncondTimeout->addTransition(this,               &wicketFSM::set_FSM_to_onCheck, OnCheck); //Для возможности быстро чекнуть билет на выход не дожидаясь таймаута на досмотр
        Drop->addTransition(Ready);

        //=================================================================================================
        connect(Ready,         &QState::entered, wrong_light_timer, &QTimer::stop );
        connect(Ready,         &QState::entered, wait_pass_timer,   &QTimer::stop );
        //=================================================================================================
        //connect(_wicketReady,         &QState::exited,  [=](){on_check = true;});
        //=================================================================================================
        connect(OnCheck,       SIGNAL(entered()), _db_Timeout_timer, SLOT(start()));
        connect(OnCheck,       &QState::exited,   _db_Timeout_timer, &QTimer::stop );
        //=================================================================================================
        //=================================================================================================
        connect(Wrong,         SIGNAL(entered()), wrong_light_timer, SLOT(start()) );
        connect(Wrong,       &QState::exited,   _db_Timeout_timer, &QTimer::stop );
        //=================================================================================================
        connect(Entry,         SIGNAL(entered()), wait_pass_timer, SLOT(start()) );  //вошли в состояния прохода, открыли калитку, зажгли лампы
        connect(entryPassed,   &QState::entered,  wait_pass_timer, &QTimer::stop );
        //=================================================================================================
        connect(Exit,          SIGNAL(entered()), wait_pass_timer, SLOT(start()) );
        connect(exitPassed,    &QState::entered,  wait_pass_timer, &QTimer::stop );
        //=================================================================================================
//=================================================================================================
        connect(UncondTimeout, SIGNAL(entered()), UncondTimeoutTimer, SLOT(start()) );

    }
    void timer_wrapper()
    {

    }
signals:
    void set_FSM_to_entry();
    void set_FSM_to_exit();
    void set_FSM_passed();
    void set_FSM_to_wrong();

    void set_FSM_to_onCheck();

private:
    void set_timer()
    {
        wait_pass_timer = new QTimer(this);
        wrong_light_timer = new QTimer(this);
        UncondTimeoutTimer = new QTimer(this);
        _db_Timeout_timer = new QTimer(this);

        _db_Timeout_timer->setSingleShot(true);
        wait_pass_timer->setSingleShot(true);
        wrong_light_timer->setSingleShot(true);
        UncondTimeoutTimer->setSingleShot(true);

        UncondTimeoutTimer->setInterval(UnconditionalTimeout_TIME);
        wait_pass_timer->setInterval(pass_wait_time);
        wrong_light_timer->setInterval(wrong_light_TIME);
        _db_Timeout_timer->setInterval(15000);



    }



    QTimer *wait_pass_timer;
    int pass_wait_time = 10000;
    QTimer *wrong_light_timer;
    int wrong_light_TIME = 2000;
    QTimer *_db_Timeout_timer;
    QTimer *UncondTimeoutTimer;
    int UnconditionalTimeout_TIME = 2000;
};

#endif // WICKETFSM_H
