#include "w2.wicketlocker.h"

wicketLocker::wicketLocker(_reader_type *reader_type,
                           dir_type *direction_type,
                           bool *ready_state_flag,
                           bool *uncond_state_flag,
                           QState *parent):
    QState(parent),
    ready_state_flag(ready_state_flag),
    uncond_state_flag(uncond_state_flag),
    reader_type(reader_type),
    direction_type(direction_type)
{
    SetArmed    = new QState( this );
//    Armed       = new wicketFSM(reader_type,
//                                direction_type,
//                                ready_state_flag,
//                                uncond_state_flag,
//                                this);

    Armed       = new checkCovidCert(reader_type,
                                   direction_type,
                                   ready_state_flag,
                                   uncond_state_flag,
                                   this);

    connect(Armed, &checkCovidCert::showState, this, &wicketLocker::showState);
    connect(Armed, &checkCovidCert::send_to_server, this, &wicketLocker::send_to_server);

    SetUnLocked = new QState( this );
    UnLocked    = new QState( this );

    //это для безусловного сброса машины  по сигналу с сервера (что бы гасить лампы)
    Armed->addTransition( this, &wicketLocker::from_server_setArmed, Armed );
    //==   Это для принудительной установки состояния если турникет вернет такой сигнал =============
    //== например при старте в разблокированном состоянии ===========================================
    //== и сохраним для возможности удалить в случае если это ПОДЧИНЕННЫЙ считыватель ===============
    ArmedToUnLockedTransition = Armed->addTransition( Armed->TicketHandler->wicket, &nikiret::unlocked, UnLocked );
    UnLockedToArmedTransition = UnLocked->addTransition( Armed->TicketHandler->wicket, &nikiret::armed, Armed );
    ///================================================================================
    // тут пошел нормальный цикл состояний// через сеттеры
    SetUnLockedToUnLockedTransition =        //сохраним для возможности удалить в случае если это ПОДЧИНЕННЫЙ считыватель
            SetUnLocked->addTransition( Armed->TicketHandler->wicket, &nikiret::unlocked, UnLocked);    // тут ждем когда разблокируется по сигналу с кросборды

    UnLocked->addTransition(this, &wicketLocker::from_server_setArmed, SetArmed);    // тут подаем кросборде сигнал на взведение турникета
    SetArmedToArmedTransition =        //сохраним для возможности удалить в случае если это ВЫХОДНОЙ считыватель
            SetArmed->addTransition(Armed->TicketHandler->wicket, &nikiret::armed, Armed);            // тут ждем когда турникет будет готов по сигналу с кросборды
    Armed->addTransition(this, &wicketLocker::from_server_setUnLocked, SetUnLocked); // тут подаем кросборде сигнал на разблокирование турникета

    //================================================================================
    connect(Armed,    &QState::entered,           this, &wicketLocker::processing_Armed_);
    connect(UnLocked, &QState::entered,          this, &wicketLocker::processing_UnLocked);

    connect(SetArmed,    &QState::entered, Armed->TicketHandler->wicket, &nikiret::lock_unlock_sequence);
    connect(SetUnLocked, &QState::entered, Armed->TicketHandler->wicket, &nikiret::lock_unlock_sequence);

    connect(Armed->TicketHandler, &wicketFSM::send_to_server, this, &wicketLocker::send_to_server);

    setInitialState( Armed );

    //    qDebug() << "wicketLocker::wicketLocker" << this->machine()->configuration() ;
    //   emit from_server_setArmed();
}

void wicketLocker::set_type_Main()  // Блажь, врядли на лету будет нужда..
{
    Armed->TicketHandler->set_type_Main();
    // Не забудь тут вернуть вынутые транзиции
}

void wicketLocker::set_type_Slave()                          //Переключаем в подчиненный режим
{

    //======= И отключим состояние досмотр охраной =======
    Armed->TicketHandler->set_type_Slave();

    //==================== отключим кросборду от автомата =================================
    Armed->removeTransition( ArmedToUnLockedTransition );
    UnLocked->removeTransition( UnLockedToArmedTransition );

    SetUnLocked->removeTransition( SetUnLockedToUnLockedTransition);
    SetArmed->removeTransition( SetArmedToArmedTransition );

    SetUnLocked->addTransition( UnLocked ); //делаем их безусловными что бы серверные сигналы не переключать
    SetArmed->addTransition( Armed );

    //emit from_crsbrd_armed(); // гарантированно взводим
}

void wicketLocker::fromServer(command cmd)
{
    switch (cmd) {
    //case command::set_Ready  : emit from_server_setArmed();    break;//безусловная установка родительское состояние что бы из любого состояния получить рэди
    //case command::set_Armed  : emit from_server_setArmed();    break;
    //case command::set_Unlock : emit from_server_setUnLocked(); break;

    default: Armed->TicketHandler->fromServer(cmd); break;
    }
}

void wicketLocker::processing_Armed_()
{
    //emit send_to_server(message(MachineState::onArmed));
    qDebug() << "Armed";
}

void wicketLocker::processing_UnLocked()
{
    emit send_to_server(message(MachineState::onUnlocked));
    emit showState(showStatus::pict_access, "");
    Armed->TicketHandler->wicket->setGREEN();
    qDebug() << "Unlocked";
}
