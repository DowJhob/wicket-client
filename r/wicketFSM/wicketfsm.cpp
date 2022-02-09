#include "wicketfsm.h"

wicketFSM::wicketFSM( _reader_type *reader_type, dir_type *direction_type, bool *ready_state_flag, bool *uncond_state_flag, QState *parent):
    QState(parent),
    ready_state_flag(ready_state_flag),
    uncond_state_flag(uncond_state_flag),
    reader_type(reader_type),
    direction_type(direction_type)
{
    wicket_init();
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
    FinalState            = new QFinalState(this);

    setInitialState( Ready );
    //================================================================================================================================================================================
    set_timer();
    addTransitions();
    //addEventTransitions();
    //=================================================================================================

    //=================================================================================================
    //connect(                               wicket, &nikiret::passed, this, &wicketFSM::from_crossboard_to_passed);   //по сигналу прохода от турникета перейдем в состояние проход

    setActions();
}

void wicketFSM::wicket_init()
{
    wicket = new nikiret();
    //connect(wicket, &nikiret::temp, this, &controller::send_state);

    connect(wicket, &nikiret::temp, this, [&](QString temp){emit send_to_server(message(MachineState::undef, command::getTemp, temp));});

    wicket->start();
}

void wicketFSM::set_timer()
{
    wait_pass_timer = new QTimer(this);
    wrong_light_timer = new QTimer(this);
    uncondDelayTimer = new QTimer(this);
    _db_Timeout_timer = new QTimer(this);
    totalWaitTimer = new QTimer(this);

    _db_Timeout_timer->setSingleShot(true);
    wait_pass_timer->setSingleShot(true);
    wrong_light_timer->setSingleShot(true);
    uncondDelayTimer->setSingleShot(true);
    totalWaitTimer->setSingleShot(true);

    //uncondDelayTimer->setInterval(uncondDelay);
    set_uncondDelay_time();
    wait_pass_timer->setInterval(pass_wait_time);
    wrong_light_timer->setInterval(wrong_light_TIME);
    _db_Timeout_timer->setInterval(15000);
    totalWaitTimer->setInterval(15000);

    connect(                         OnCheckEntry, &QState::exited, _db_Timeout_timer, &QTimer::stop );
    //=================================================================================================
    connect(                          OnCheckEXit, &QState::exited, _db_Timeout_timer, &QTimer::stop );
    //=================================================================================================
    connect(                                Wrong, &QState::exited, _db_Timeout_timer, &QTimer::stop );
    //=================================================================================================
    connect(                                Entry, &QState::exited, wait_pass_timer, &QTimer::stop );
    //=================================================================================================
    connect(                                 Exit, &QState::exited, wait_pass_timer, &QTimer::stop );
}

void wicketFSM::addTransitions()
{
    Ready->addTransition(                    this, &wicketFSM::from_server_to_wrong, Wrong); // по сигналу с сервера переходим в состояния запрещено
    Ready->addTransition(                    this, &wicketFSM::from_reader_to_onCheckEntry, OnCheckEntry); //состояние на проверке
    Ready->addTransition(                    this, &wicketFSM::from_reader_to_onCheckEXit, OnCheckEXit); //состояние на проверке

    Wrong->addTransition(       wrong_light_timer, &QTimer::timeout, FinalState); //по сигналу таймера тоже обратно
    dbTimeout->addTransition( Wrong);

    OnCheckEntry->addTransition(             this, &wicketFSM::from_server_to_wrong, Wrong); //wrong ticket
    OnCheckEntry->addTransition(             this, &wicketFSM::from_server_to_entry, Entry); // по сигналу с сервера переходим в состояние открыто на вход
    OnCheckEntry->addTransition(_db_Timeout_timer, &QTimer::timeout,             dbTimeout);
    //=================================================================================================

    OnCheckEXit->addTransition(              this, &wicketFSM::from_server_to_wrong, Wrong); //wrong ticket
    OnCheckEXit->addTransition(              this, &wicketFSM::from_server_to_exit,  Exit); // по сигналу с сервера переходим в состояние открыто на выход
    OnCheckEXit->addTransition( _db_Timeout_timer, &QTimer::timeout,             dbTimeout);
    //=================================================================================================

    Entry->addTransition(         wait_pass_timer, &QTimer::timeout,             Drop); //по сигналу таймера в состояние сброс прохода
    Entry->addTransition(                  wicket, &nikiret::passed,   entryPassed); //по сигналу прохода от турникета перейдем в состояние проход
    Entry->addTransition(                    this, &wicketFSM::from_server_to_entryPassed,   entryPassed);
    entryPassed->addTransition( UncondTimeout );           // и сразу в задержку прохода по пути отослав данные на сервер
    //=================================================================================================

    Exit->addTransition(          wait_pass_timer, &QTimer::timeout,               Drop);
    Exit->addTransition(                   wicket, &nikiret::passed,     exitPassed);
    Exit->addTransition(                     this, &wicketFSM::from_server_to_exitPassed,     exitPassed);
    exitPassed->addTransition( FinalState );

    unCondToReady =
            UncondTimeout->addTransition(uncondDelayTimer, &QTimer::timeout,                   FinalState);
    unCondToCheckExit =
            UncondTimeout->addTransition(    this, &wicketFSM::from_reader_to_onCheckEXit, OnCheckEXit); //Для возможности быстро чекнуть билет на выход не дожидаясь таймаута на досмотр
    Drop->addTransition(FinalState);
    //=================================================================================================
    Ready->addTransition(          totalWaitTimer, &QTimer::timeout, FinalState); //по сигналу таймера тоже обратно
}

void wicketFSM::setActions()
{
    //============= processing ===================================
    connect(Ready,         &QState::entered, this, &wicketFSM::processing_Ready);
    connect(Ready,         &QState::exited,  this, &wicketFSM::processing_exReady);
    connect(OnCheckEntry,  &QState::entered, this, &wicketFSM::processing_onCheckEntry);
    connect(OnCheckEXit,   &QState::entered, this, &wicketFSM::processing_onCheckExit);
    connect(dbTimeout,     &QState::entered, this, &wicketFSM::processing_dbTimeout);
    connect(Wrong,         &QState::entered, this, &wicketFSM::processing_Wrong);
    connect(Entry,         &QState::entered, this, &wicketFSM::processing_Entry);  //вошли в состояния прохода, открыли калитку, зажгли лампы
    connect(Exit,          &QState::entered, this, &wicketFSM::processing_Exit);
    connect(entryPassed,   &QState::entered, this, &wicketFSM::processing_EntryPassed);   // Когда входим в это состояние отсылаем сообщение на сервер
    connect(exitPassed,    &QState::entered, this, &wicketFSM::processing_ExitPassed);
    connect(Drop,          &QState::entered, this, &wicketFSM::processing_Drop);

    connect(UncondTimeout, &QState::entered, this, &wicketFSM::processing_UncondTimeout);
    connect(UncondTimeout, &QState::exited,  this, &wicketFSM::processing_exUncondTimeout);
    ///-----------------------------------------------------
}

void wicketFSM::set_type_Main()
{
    if(unCondToReadySlaveType != nullptr)
        UncondTimeout->removeTransition(unCondToReadySlaveType);// Удаляю безусловный переход
    //     unCondToReady->setTargetState(UncondTimeout);           // Возвращаю условные переходы
    //    unCondToCheckExit->setTargetState(UncondTimeout);
}

void wicketFSM::set_type_Slave()
{
    //  UncondTimeout->removeTransition(unCondToCheckExit);
    //  UncondTimeout->removeTransition(unCondToReady);
    unCondToReadySlaveType = UncondTimeout->addTransition(Ready); // Не дожидаясь таймера сразу в Ready
    //qDebug() << unCondToReadySlaveType << UncondTimeout->transitions();
}

void wicketFSM::set_uncondDelay_time(int uncondDelay)
{
    uncondDelayTimer->setInterval(uncondDelay);
}

void wicketFSM::fromServer(command cmd)
{
    switch (cmd) {
    //case command::set_Wrong                   : emit from_server_to_wrong ();     break;
    //case command::set_EntryOpen               : emit from_server_to_entry();      break;
    //case command::set_ExitOpen                : emit from_server_to_exit();       break;
    default: break;
    }
}

void wicketFSM::fromServerState(MachineState state)
{
    switch (state) {
    //========== синхронизация ведущего - подчиненного ==============
    case MachineState::onWrongRemote    :
        if(*reader_type == _reader_type::slave) // если это главный считыватель то ему пофигу что подчиненный не отработал статус на проверке
        {
            emit from_server_to_wrong(); // не получилось чекнуть билет на выход
        }break;
        //========== остальные просто исполняем ==============
    case MachineState::onWrong                : emit from_server_to_wrong();       break;
    case MachineState::onEntry                : emit from_server_to_entry();       break; // тут зажгутся лампы у ведомого
    case MachineState::onExit                 : emit from_server_to_exit();        break;
    case MachineState::onEtryPassed           : emit from_server_to_entryPassed(); break;
    case MachineState::onExitPassed           : emit from_server_to_exitPassed();  break;
    default                                   :                                    break;
    }
}

void wicketFSM::set_onCheckEntry()
{
    emit from_reader_to_onCheckEntry();
}

void wicketFSM::set_onCheckEXit()
{
    emit from_reader_to_onCheckEXit();
}

void wicketFSM::send_state2(message msg)
{
    if(*reader_type == _reader_type::_main)
        emit send_to_server(msg);
}

void wicketFSM::processing_dbTimeout()
{
    showState(showStatus::pict_denied, "Ошибка базы данных" );
    wicket->setRED();
    send_state2(message(MachineState::on_dbTimeout, command::undef));
    qDebug() << "dbTimeout";
}

void wicketFSM::processing_Wrong()
{
    wrong_light_timer->start();
    showState(showStatus::pict_denied, cmd_arg );
    wicket->setRED();
    send_state2(message(MachineState::onWrong, command::undef, cmd_arg));
    qDebug() << "Wrong";
}

void wicketFSM::processing_Ready()
{
    wrong_light_timer->stop();
    wait_pass_timer->stop();
    totalWaitTimer->start();

    *ready_state_flag = true;
    wicket->setLightOFF();
    showState(showStatus::Ready, "");
    send_state2(message(MachineState::onReady, command::undef));
    qDebug() << "Ready";
}

void wicketFSM::processing_exReady()
{
    *ready_state_flag = false;
}

void wicketFSM::sub_processing_onCheck(MachineState state)
{
    _db_Timeout_timer->start();
    wicket->alarm();
    message msg;
    switch (*reader_type)
    {
    case _reader_type::_main : msg = message(state, command::undef, cmd_arg); break;
    case _reader_type::slave : msg = message(MachineState::getRemoteBarcode, command::undef, cmd_arg); break;
    }
    emit send_to_server(msg);
    showState(showStatus::pict_onCheck, "");
    qDebug() << "onCheck: " << static_cast<int>(state);
}

void wicketFSM::processing_onCheckEntry()
{
    sub_processing_onCheck(MachineState::onCheckEntry);
}

void wicketFSM::processing_onCheckExit()
{
    sub_processing_onCheck(MachineState::onCheckExit);
}

void wicketFSM::processing_Entry()
{
    wait_pass_timer->start();
    if(*direction_type == dir_type::entry)
    {
        wicket->setGREEN();
        showState(showStatus::pict_access, "");
    }
    else if(*direction_type == dir_type::exit_)
    {
        wicket->setRED();
        showState(showStatus::pict_denied, "Подождите, вам навстречу уже кто-то идет.");
    }
    wicket->set_turnstile_to_pass(dir_type::entry);
    send_state2(message(MachineState::onEntry, command::undef));
    qDebug() << "Entry";
}

void wicketFSM::processing_Exit()
{
    wait_pass_timer->start();
    if(*direction_type == dir_type::entry)
    {
        wicket->setRED();
        showState(showStatus::pict_denied, "Подождите, вам навстречу уже кто-то идет.");
    }
    else if(*direction_type == dir_type::exit_)
    {
        wicket->setGREEN();
        showState(showStatus::pict_access, "");
    }
    wicket->set_turnstile_to_pass(dir_type::exit_);
    send_state2(message(MachineState::onExit, command::undef));
    qDebug() << "Exit";
}

void wicketFSM::processing_EntryPassed()
{
    send_state2(message(MachineState::onEtryPassed, command::undef));
    qDebug() << "EntryPassed";
}

void wicketFSM::processing_ExitPassed()
{
    send_state2(message(MachineState::onExitPassed, command::undef));
    qDebug() << "ExitPassed";
}

void wicketFSM::processing_UncondTimeout()
{
    uncondDelayTimer->start();
    *uncond_state_flag = true;
    showState(showStatus::DbTimeout, "");
    wicket->setRED();
    send_state2(message(MachineState::onUncodTimeout, command::undef));
    qDebug() << "UncondTimeout";
}

void wicketFSM::processing_exUncondTimeout()
{
    uncondDelayTimer->stop();
    *uncond_state_flag = false;
}

void wicketFSM::processing_Drop()
{
    send_state2(message(MachineState::onPassDropped, command::undef));
    qDebug() << "Drop";
}

//void wicketFSM::setEventTransition(QState *source, command type, QState *target)
//{
//    QEventTransition *Transition = new QEventTransition(this, static_cast<QEvent::Type>(QEvent::User + static_cast<int>(type) ));
//    Transition->setTargetState(target);
//    source->addTransition(Transition);
//}

//void wicketFSM::addEventTransitions()
//{
//    //========= по сигналу с сервера переходим в состояние запрещено ====================================
//    setEventTransition(Ready, command::set_Wrong, Wrong);

//    //========= состояние на проверке ===============================================
//    setEventTransition(Ready, command::set_CheckEntry, OnCheckEntry);

//    //========= состояние на проверке =============================================================================
//    setEventTransition(Ready, command::set_CheckExit, OnCheckEXit);

//    //========= по сигналу таймера тоже обратно ===========================================================================
//    Wrong->addTransition(       wrong_light_timer, &QTimer::timeout, FinalState);
//    dbTimeout->addTransition( Wrong);

//    //========= wrong ticket =======================================================================================
//    setEventTransition(OnCheckEntry, command::set_Wrong, Wrong);

//    //========== по сигналу с сервера переходим в состояние открыто на вход=======================================================================================
//    setEventTransition(OnCheckEntry, command::set_EntryOpen, Entry);

//    //=================================================================================================
//    OnCheckEntry->addTransition(_db_Timeout_timer, &QTimer::timeout,             dbTimeout);

//    //========= wrong ticket =======================================================================================
//    setEventTransition(OnCheckEXit, command::set_Wrong, Wrong);

//    //========= по сигналу с сервера переходим в состояние открыто на выход ========================================================================================
//    setEventTransition(OnCheckEXit, command::set_ExitOpen, Exit);

//    //=================================================================================================
//    OnCheckEXit->addTransition( _db_Timeout_timer,  &QTimer::timeout,             dbTimeout);

//    //========== по сигналу таймера в состояние сброс прохода =======================================================================================
//    Entry->addTransition(         wait_pass_timer, &QTimer::timeout,             Drop);

//    //========== по сигналу прохода от турникета перейдем в состояние проход =====================================================================
//    Entry->addTransition(                    wicket, &nikiret::passed,   entryPassed);

//    //========== for slave reader ======================================================================================
//    setEventTransition(Entry, command::set_Ready, Ready);

//    //========== и сразу в задержку прохода по пути отослав данные на сервер ======================================================================================
//    entryPassed->addTransition( UncondTimeout );

//    //=================================================================================================
//    Exit->addTransition(          wait_pass_timer, &QTimer::timeout,               Drop);
//    Exit->addTransition(                     wicket, &nikiret::passed,     exitPassed);

//    //=================================================================================================
//    setEventTransition(Exit, command::set_Ready, Ready);

//    //=================================================================================================
//    exitPassed->addTransition( FinalState );

//    unCondToReady =
//            UncondTimeout->addTransition(uncondDelayTimer, &QTimer::timeout,                   FinalState);

//    //========== Для возможности быстро чекнуть билет на выход не дожидаясь таймаута на досмотр =========================================================================
//    setEventTransition(UncondTimeout, command::set_CheckExit, OnCheckEXit);

//    //=================================================================================================
//    Drop->addTransition(FinalState);

//    //=========== по сигналу таймера тоже обратно ======================================================================================
//    Ready->addTransition(          totalWaitTimer, &QTimer::timeout, FinalState); //
//}
