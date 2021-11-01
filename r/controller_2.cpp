#include "controller_2.h"

controller::controller()
{ }

controller::~controller() //Q_DECL_OVERRIDE
{
    //        if(testt != nullptr)
    //            testt->deleteLater();
    //        if(testt_pass != nullptr)
    //            testt_pass->deleteLater();
    //        if(machine != nullptr)
    //            machine->deleteLater();
    //        if(serverSearch != nullptr)
    //            serverSearch->deleteLater();
    //        if(wicket != nullptr)
    //            wicket->deleteLater();
    //        if(serverFound != nullptr)
    //            serverFound->deleteLater();
}

void controller::start()
{
    t = new QElapsedTimer;
    wicket_init();
    set_timer();
    //==================================================================================================================
    machine = new mainFSM(wicket,
                          &reader_type,
                          &direction_type,
                          &ready_state_flag,
                          &uncond_state_flag,
                          this);

    connect(this, &controller::ext_provided_server_searchSIG, machine, &mainFSM::serverSearchSIG);
    connect(this, &controller::ext_provided_network_readySIG, machine, &mainFSM::serverFoundSIG);

    ///-----------------------------------------------------
    //==================================================================================================================
    //_updater.setPort( FILE_TRANSFER_PORT );
    //_updater.setFilename( QCoreApplication::applicationFilePath() );
    //connect(&_updater, &updater::ready, [ & ](){ emit exit(42); } );
    //==================================================================================================================
    connect(machine, &mainFSM::send_to_server, this, &controller::send_to_server);
    machine->start();


    //set_reverse();   // для теста!!!!
}

void controller::new_cmd_parse(message msg)
{
    cmd_arg = msg.body.toString();
    if (msg.cmd != command::undef)
        switch (msg.cmd) {
        //  безусловные команды
        case command::set_test       : emit from_server_set_test();      break;
        case command::set_normal     : emit from_server_set_normal();    break;
        case command::set_iron_mode  : emit from_server_set_iron_mode(); break;
        case command::set_type_entry : set_type_Entry();            break;
        case command::set_type_exit  : set_type_Exit();             break;

        default                      : machine->fromServer(msg.cmd, msg.state); break;
        }
    // проксированные статусы
    if (msg.state != MachineState::undef)
        switch (msg.state) {
        //========== синхронизация ведущего - подчиненного ==============
        case MachineState::getRemoteBarcode : remote_barcode(cmd_arg);     break; // прокси команда от подчиненного
            //========== остальные просто исполняем ==============
        default                             : machine->fromServerState(msg.state);                      break;
        }
}

void controller::remote_barcode(QString bc) // Дергается удаленно
{
    if ( ready_state_flag || uncond_state_flag )
    {
        (machine->*remote_onCheck_handler)();
    }
    else
        emit send_to_server(message(MachineState::onWrongRemote, command::undef, "Ведущий считыватель\nне готов" ));
}

void controller::local_barcode(QByteArray data)
{
    qDebug() << "local_barcode: " + data;
    cmd_arg = data;
    t->start();
    if ( ready_state_flag )  // наверное избыточная проверка, и так в онЧек можно только из рэди
    {
        (machine->*local_onCheck_handler)(); //emit  predefined signal set_onCheckEntry or set_onCheckEXit;

        if (!test_state_flag)
            wicket->alarm();
        //if(iron_mode_flag)
        //{
        //    (this->*handler_opener)();   //https://stackoverflow.com/questions/26331628/reference-to-non-static-member-function-must-be-called
        //    emit send_to_server(message(msg_type::command, command::iron_bc, data ));
        //}
        //else
        //    emit send_to_server(message(msg_type::command, command::barcode, data ));
    }
}

void controller::send_state2(message msg)
{
    if(reader_type == _reader_type::_main)
        emit send_to_server(msg);
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

void controller::set_type_Entry()
{
    direction_type = dir_type::entry;
    local_onCheck_handler = &mainFSM::set_onCheckEntry; //дергается считывателем  шк
    remote_onCheck_handler = &mainFSM::set_onCheckEXit; // дергается сервером по команде удаленного считывателя
    qDebug() << "Entry";
}

void controller::set_type_Exit()
{
    direction_type = dir_type::exit_;
    local_onCheck_handler = &mainFSM::set_onCheckEXit; // дергается сервером по команде удаленного считывателя
    remote_onCheck_handler = &mainFSM::set_onCheckEntry; //дергается считывателем  шк
    qDebug() << "Exit";
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

void controller::wicket_init()
{
    wicket = new nikiret();
    //connect(wicket, &nikiret::temp, this, &controller::send_state);

    connect(wicket, &nikiret::temp, this, [&](QString temp){emit send_to_server(message(MachineState::undef, command::getTemp, temp));});

    wicket->start();
}

void controller::send_state(QString temp)
{
    emit send_to_server(message(MachineState::undef, command::getTemp, temp));
}
