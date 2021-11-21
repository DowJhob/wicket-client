#include "controller_2.h"

controller::controller()
{ }

void controller::start()
{
    t = new QElapsedTimer;
    wicket_init();
    //set_timer();
    //==================================================================================================================


    ///-----------------------------------------------------
    //==================================================================================================================
    //_updater.setPort( FILE_TRANSFER_PORT );
    //_updater.setFilename( QCoreApplication::applicationFilePath() );
    //connect(&_updater, &updater::ready, [ & ](){ emit exit(42); } );
    //==================================================================================================================


    //set_reverse();   // для теста!!!!
}

void controller::wicket_init()
{
    wicket = new nikiret();
    //connect(wicket, &nikiret::temp, this, &controller::send_state);

    connect(wicket, &nikiret::temp,     this, [&](QString temp){emit send_to_server(message(MachineState::undef, command::getTemp, temp));});
    connect(wicket, &nikiret::armed,    this, [&](){emit send_to_server(message(MachineState::undef, command::getArmed));});
    connect(wicket, &nikiret::unlocked, this, [&](){emit send_to_server(message(MachineState::undef, command::getUnlock));});
    connect(wicket, &nikiret::passed,   this, [&](){emit send_to_server(message(MachineState::undef, command::getPassed));});

    wicket->start();
}

void controller::new_cmd_parse(message msg)
{
    cmd_arg = msg.body.toString();
        switch (msg.cmd) {
        //  безусловные команды
        case command::setArmed                 : wicket->lock_unlock_sequence();    break;
        case command::setUnlock                : wicket->lock_unlock_sequence();    break;
        case command::setEntryOpen             : wicket->set_turnstile_to_pass(dir_type::entry); break;          // Открываем турникет
        case command::setExitOpen              : wicket->set_turnstile_to_pass(dir_type::exit_); break;            //

        case command::setGreenLampOn           : wicket->setGREEN(); break;
        case command::setRedLampOn             : wicket->setRED(); break;
        case command::setLampOff               : wicket->setLightOFF();  break;             // Отправляем команду погасить лампы

        case command::setAlarm                 : wicket->alarm(); break;              // Бибип

            // Показываем картинку с текстом на эkране считывателя
        case command::showServiceStatus        :       // Турникет не готов и все такое
        case command::showReadyStatus          :         // Турникет готов, покажите билет или ковид куар
        case command::showOpenStatus           :          // Пжалста проходите, зелЁни стралачка

        case command::showPlaceStatus      :                   // синий? фон со стрелкой куда пихать
        case command::showCheckStatus      :                   // оранжевый фон с часиками
        case command::showFailStatus      :                      // Красный крестик

       // case command::showDbWaitStatus         :       // Подождите проверяем билет, обычно не успевают увидеть
//        case command::showWaitStatus           :         // Подождите вам навстречу уже кто то идет
//        case command::showSecurityCheckStatus  :      // Подождите охрана проверяет предыщего посетителя

//        case command::showTicketFailStatus     :    // Wrong ticket и что пошло не так
//        case command::showDbTimeoutStatus      :     // База данных не отвечает
//        case command::showDoubleScanFailStatus :
            emit s_showStatus(msg); break;

        default :
            //machine->postEvent(new QEvent( static_cast<QEvent::Type>(QEvent::User + static_cast<int>(msg.cmd) ) ));
            break;
        }

}

void controller::local_barcode(QByteArray data)
{
    t->start();
    emit send_to_server(message(MachineState::undef, command::getBarcode, data));
    qDebug() << "local_barcode: " + data;
}

void controller::from_server_set_test()
{
    //=================================== TEST TIMER =====================================================
    qDebug() << "set test ==============================================================================TEST";
    test_state_flag = true;
    testt->start(3000);

    if(reader_type == _reader_type::_main )
    {
        //connect(testt_pass, &QTimer::timeout,      serverFound->Armed, &wicketFSM::set_FSM_passed);
        //connect(serverFound->Armed->Entry, &QState::entered,   this, &controller::timer_wrapper);
        //connect(serverFound->Armed->Exit,  &QState::entered,   this, &controller::timer_wrapper);
    }
}

void controller::from_server_set_normal()
{
    //=================================== TEST TIMER =====================================================
    qDebug() << "set normal==============================================================================NORMAL";
    test_state_flag = false;
    iron_mode_flag = false;
    testt->stop();
    testt_pass->stop();

    //disconnect(testt_pass, &QTimer::timeout,      serverFound->Armed, &wicketFSM::set_FSM_passed);


    //disconnect(serverFound->Armed->Entry, &QState::entered,   this, &controller::timer_wrapper);
    //disconnect(serverFound->Armed->Exit,  &QState::entered,   this, &controller::timer_wrapper);
}

void controller::timer_wrapper()
{
    qDebug()<<"wrapper";
    testt_pass->start();
}

void controller::from_server_set_iron_mode()
{
    qDebug() << "set iron ==============================================================================IRON";
    iron_mode_flag = true;
}

void controller::set_timer()
{
    testt_pass = new QTimer(this);
    testt_pass->setInterval(1000);
    testt_pass->setSingleShot(true);


    testt = new QTimer(this);
    testt->setInterval(3000);
    //       connect(testt, &QTimer::timeout, [=](){ send_barcode("9780201379624");} );
    connect(testt, &QTimer::timeout, [=](){
        QByteArray b1 = "superticket";
        QByteArray b2 = "forbidticket";
        if (test_flag)
            local_barcode(b1);
        else
            local_barcode(b2);
        test_flag = !test_flag;
    } );
}
