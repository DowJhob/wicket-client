#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include <QProcess>
#include <QStateMachine>
#include <QTime>

#include <QThread>
#include <QElapsedTimer>

#include <common_types.h>

#include <nikiret.h>
#include <updater.h>
#include <wicketFSM/wicketfsm.h>
#include <wicketFSM/wicketlocker.h>
#include "command.h"

#define CROSSBOARD_SERIAL_PORT_NAME     "/dev/ttyAPP3"
#define FILE_TRANSFER_PORT              27008

class controller: public QObject
{
    Q_OBJECT
public:
    _reader_type reader_type = _reader_type::_main;
    dir_type direction_type = dir_type::entry;
    controller()
    { }
    ~controller() //Q_DECL_OVERRIDE
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

    wicketLocker *serverFound;
    QElapsedTimer *t;
public slots:
    void start()
    {
        t = new QElapsedTimer;
        wicket_init();
        set_timer();
        //==================================================================================================================
        machine = new QStateMachine(this);
        serverSearch = new QState( machine );                                   // поиск сети
        // это состояние нужно для показа картинки (а может достаточно ИП) состояния на дисплее(а может и не нужно)
        serverFound = new wicketLocker(machine); // состояние сервер найден теперь тут

        serverSearch->addTransition(this, &controller::ext_provided_network_readySIG, serverFound);//// Из любого состояния можно перейти в проебана сеть
        serverFound->addTransition(this, &controller::ext_provided_server_searchSIG, serverSearch);//// Из любого состояния можно перейти в проебана сеть

        connect(serverSearch, &QState::entered, this, &controller::processing_serverSearch); // тут покажем картинку что идет поиск (или просто включим мигание ИП
        //   connect(_wicketLocker_serverFound, &QState::entered, this, &controller::set_status_READY);   //вызовем слот  для показа картинок по сути

        machine->setInitialState( serverSearch );
        ///--------------------------- Машина сброса-взвода прохода -----------------------------------------------
        //        connect(this,   &controller::from_server_setReady,    _wicketLocker_serverFound, &wicketLocker::in_uncond);
        connect(wicket, &nikiret::unlocked,                   serverFound, &wicketLocker::from_crsbrd_unlock);
        connect(wicket, &nikiret::armed,                      serverFound, &wicketLocker::from_crsbrd_armed);
        connect(this,   &controller::from_server_setArmed,    serverFound, &wicketLocker::from_server_setArmed);
        connect(this,   &controller::from_server_setUnLocked, serverFound, &wicketLocker::from_server_setUnLocked); // тут подаем кросборде сигнал на разблокирование турникета
        connect(this,   &controller::from_server_to_ready,    serverFound, &wicketLocker::from_server_setArmed);   //безусловная установка родительское состояние что бы из любого состояния получить рэди

        connect(serverFound->Armed,    &QState::entered,           this, &controller::processing_Armed_);
        connect(serverFound->UnLocked, &QState::entered,          this, &controller::processing_UnLocked);

        connect(serverFound->SetArmed,    &QState::entered, wicket, &nikiret::lock_unlock_sequence);
        connect(serverFound->SetUnLocked, &QState::entered, wicket, &nikiret::lock_unlock_sequence);
        ///--------------------------- Машина состояний проходов -----------------------------------------------
        connect(this,   &controller::from_server_to_wrong,  serverFound->Armed, &wicketFSM::set_FSM_to_wrong);  // по сигналу с сервера переходим в состояния запрещено
        connect(this,   &controller::from_server_to_entry,  serverFound->Armed, &wicketFSM::set_FSM_to_entry);  // по сигналу с сервера переходим в состояние открыто
        connect(this,   &controller::from_server_to_exit,   serverFound->Armed, &wicketFSM::set_FSM_to_exit);
        connect(wicket, &nikiret::passed,                   serverFound->Armed, &wicketFSM::set_FSM_passed);   //по сигналу прохода от турникета перейдем в состояние проход

        connect(this, &controller::from_server_to_entryPassed, serverFound->Armed, &wicketFSM::set_FSM_EntryPassed);
        connect(this, &controller::from_server_to_exitPassed, serverFound->Armed, &wicketFSM::set_FSM_ExitPassed);

        connect(this,   &controller::set_onCheckEntry,      serverFound->Armed, &wicketFSM::set_FSM_to_onCheckEntry);
        connect(this,   &controller::set_onCheckEXit,       serverFound->Armed, &wicketFSM::set_FSM_to_onCheckEXit);

        //        connect(this,   &controller::from_server_onCheckEntry, serverFound->Armed, &wicketFSM::set_FSM_to_onCheckEntry);
        //       connect(this,   &controller::from_server_onCheckEXit, serverFound->Armed, &wicketFSM::set_FSM_to_onCheckEXit);

        //============= processing ===================================
        connect(serverFound->Armed->Ready,         &QState::entered, this, &controller::processing_Ready);
        connect(serverFound->Armed->Ready,         &QState::exited,  this, &controller::processing_exReady);
        connect(serverFound->Armed->OnCheckEntry,  &QState::entered, this, &controller::processing_onCheckEntry);
        connect(serverFound->Armed->OnCheckEXit,   &QState::entered, this, &controller::processing_onCheckExit);
        connect(serverFound->Armed->dbTimeout,     &QState::entered, this, &controller::processing_dbTimeout);
        connect(serverFound->Armed->Wrong,         &QState::entered, this, &controller::processing_Wrong);
        connect(serverFound->Armed->Entry,         &QState::entered, this, &controller::processing_Entry);  //вошли в состояния прохода, открыли калитку, зажгли лампы
        connect(serverFound->Armed->Exit,          &QState::entered, this, &controller::processing_Exit);
        connect(serverFound->Armed->entryPassed,   &QState::entered, this, &controller::processing_EntryPassed);   // Когда входим в это состояние отсылаем сообщение на сервер
        connect(serverFound->Armed->exitPassed,    &QState::entered, this, &controller::processing_ExitPassed);
        connect(serverFound->Armed->Drop,          &QState::entered, this, &controller::processing_Drop);

        connect(serverFound->Armed->UncondTimeout, &QState::entered, this, &controller::processing_UncondTimeout);
        connect(serverFound->Armed->UncondTimeout, &QState::exited,  this, &controller::processing_exUncondTimeout);
        ///-----------------------------------------------------
        //==================================================================================================================
        _updater.setPort( FILE_TRANSFER_PORT );
        _updater.setFilename( QCoreApplication::applicationFilePath() );
        connect(&_updater, &updater::ready, [ & ](){ emit exit(42); } );
        //==================================================================================================================
        machine->start();

        //set_reverse();   // для теста!!!!
    }
    void new_cmd_parse(message msg)
    {
        cmd_arg = msg.body.toString();
        if (msg.cmd != command::undef)
            switch (msg.cmd) {
            //     case _comm::heartbeat: ;
            //  безусловные команды
            case command::set_test                    : emit from_server_set_test();      break;
            case command::set_normal                  : emit from_server_set_normal();    break;
            case command::set_iron_mode               : emit from_server_set_iron_mode(); break;
            case command::set_type_main               : emit set_type_Main();             break;
            case command::set_type_slave              : emit set_type_Slave();            break;
            case command::set_type_entry              : emit set_type_Entry();            break;
            case command::set_type_exit               : emit set_type_Exit();             break;
            case command::set_Armed                   : emit from_server_setArmed();      break;
            case command::set_Unlock                  : emit from_server_setUnLocked();   break;

            case command::set_Ready                   : emit from_server_to_ready();      break;
            case command::set_Wrong                   : emit from_server_to_wrong ();     break;
            case command::set_EntryOpen               : emit from_server_to_entry();      break;
            case command::set_ExitOpen                : emit from_server_to_exit();       break;

            default: break;
            }
        // проксированные статусы
        if (msg.state != MachineState::undef)
            switch (msg.state) {
            //========== синхронизация ведущего - подчиненного ==============
            case MachineState::getRemoteBarcode : remote_barcode(cmd_arg);     break; // прокси команда от подчиненного
            case MachineState::onWrongRemote    :
                if(reader_type == _reader_type::slave) // если это главный считыватель то ему пофигу что подчиненный не отработал статус на проверке
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
    void remote_barcode(QString bc) // Дергается удаленно
    {
        if ( ready_state_flag || uncond_state_flag )
        {
            (this->*remote_onCheck_handler)();
        }
        else
            send_to_server(message(MachineState::onWrongRemote, command::undef, "Ведущий считыватель\nне готов" ));
    }
    void local_barcode(QByteArray data)
    {
        cmd_arg = data;
        t->start();
        if ( ready_state_flag )  // наверное избыточная проверка, и так в онЧек можно только из рэди
        {
            (this->*local_onCheck_handler)(); //emit  predefined signal set_onCheckEntry or set_onCheckEXit;

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

private slots:
    void send_state2(message msg)
    {
        if(reader_type == _reader_type::_main)
            emit send_to_server(msg);
    }
    //============= обработка поиска сервера ===================================
    void processing_serverSearch()
    {
        setPictureSIG(picture::pict_service, "");
    }
    //============= обработка сброса взвода турникета ===================================
    void processing_Armed_()
    {
        emit send_to_server(message(MachineState::onArmed));
        qDebug() << "Armed";
    }
    void processing_UnLocked()
    {
        emit send_to_server(message(MachineState::onUnlocked));
        setPictureSIG(picture::pict_access, "");
        wicket->setGREEN();
        qDebug() << "Unlocked";
    }
    //============= обработка проходов ===================================
    void processing_dbTimeout()
    {
        setPictureSIG(picture::pict_denied, "Ошибка базы данных" );
        wicket->setRED();
        send_state2(message(MachineState::on_dbTimeout, command::undef));
        qDebug() << "dbTimeout";
    }
    void processing_Wrong()
    {
        setPictureSIG(picture::pict_denied, cmd_arg );
        wicket->setRED();
        send_state2(message(MachineState::onWrong, command::undef, cmd_arg));
        qDebug() << "Wrong";
    }
    void processing_Ready()
    {
        ready_state_flag = true;
        wicket->setLightOFF();
        setPictureSIG(picture::pict_ready, "");
        send_state2(message(MachineState::onReady, command::undef));
        qDebug() << "Ready";
    }
    void processing_exReady()
    {
        ready_state_flag = false;
    }
    void sub_processing_onCheck(MachineState state)
    {
        message msg;
        switch (reader_type)
        {
        case _reader_type::_main : msg = message(state, command::undef, cmd_arg); break;
        case _reader_type::slave : msg = message(MachineState::getRemoteBarcode, command::undef, cmd_arg); break;
        }
        emit send_to_server(msg);
        setPictureSIG(picture::pict_onCheck, "");
        qDebug() << "onCheck: " << static_cast<int>(state);
    }
    void processing_onCheckEntry()
    {
        sub_processing_onCheck(MachineState::onCheckEntry);
    }
    void processing_onCheckExit()
    {
        sub_processing_onCheck(MachineState::onCheckExit);
    }
    void processing_Entry()
    {
        if(direction_type == dir_type::entry)
        {
            wicket->setGREEN();
            setPictureSIG(picture::pict_access, "");
        }
        else if(direction_type == dir_type::exit_)
        {
            wicket->setRED();
            setPictureSIG(picture::pict_denied, "Подождите, вам навстречу уже кто-то идет.");
        }
        wicket->set_turnstile_to_pass(dir_type::entry);
        send_state2(message(MachineState::onEntry, command::undef));
        qDebug() << "Entry";
    }
    void processing_Exit()
    {
        if(direction_type == dir_type::entry)
        {
            wicket->setRED();
            setPictureSIG(picture::pict_denied, "Подождите, вам навстречу уже кто-то идет.");
        }
        else if(direction_type == dir_type::exit_)
        {
            wicket->setGREEN();
            setPictureSIG(picture::pict_access, "");
        }
        wicket->set_turnstile_to_pass(dir_type::exit_);
        send_state2(message(MachineState::onExit, command::undef));
        qDebug() << "Exit";
    }
    void processing_EntryPassed()
    {
        send_state2(message(MachineState::onEtryPassed, command::undef));
        qDebug() << "EntryPassed";
    }
    void processing_ExitPassed()
    {
        send_state2(message(MachineState::onExitPassed, command::undef));
        qDebug() << "ExitPassed";
    }
    void processing_UncondTimeout()
    {
        uncond_state_flag = true;
        setPictureSIG(picture::pict_timeout, "");
        wicket->setRED();
        send_state2(message(MachineState::onUncodTimeout, command::undef));
        qDebug() << "UncondTimeout";
    }
    void processing_exUncondTimeout()
    {
        uncond_state_flag = false;
    }
    void processing_Drop()
    {
        send_state2(message(MachineState::onPassDropped, command::undef));
        qDebug() << "Drop";
    }
    //=================================== TEST MODE ====================
    void from_server_set_test()
    {
        //=================================== TEST TIMER =====================================================
        qDebug() << "set test ==============================================================================TEST";
        test_state_flag = true;
        testt->start(3000);

        if(reader_type == _reader_type::_main )
        {
            connect(testt_pass, &QTimer::timeout,      serverFound->Armed, &wicketFSM::set_FSM_passed);
            connect(serverFound->Armed->Entry, &QState::entered,   this, &controller::timer_wrapper);
            connect(serverFound->Armed->Exit,  &QState::entered,   this, &controller::timer_wrapper);
        }
    }
    void from_server_set_normal()
    {
        //=================================== TEST TIMER =====================================================
        qDebug() << "set normal==============================================================================NORMAL";
        test_state_flag = false;
        iron_mode_flag = false;
        testt->stop();
        testt_pass->stop();
        //   if(main_direction == direction_state::dir_entry )
        disconnect(testt_pass, &QTimer::timeout,      serverFound->Armed, &wicketFSM::set_FSM_passed);
        //   else

        disconnect(serverFound->Armed->Entry, &QState::entered,   this, &controller::timer_wrapper);
        disconnect(serverFound->Armed->Exit,  &QState::entered,   this, &controller::timer_wrapper);
    }
    void timer_wrapper()
    {
        qDebug()<<"wrapper";
        testt_pass->start();
    }
    //=================================== IRON MODE =====================
    void from_server_set_iron_mode()
    {
        qDebug() << "set iron ==============================================================================IRON";
        iron_mode_flag = true;
    }
    //======================================================================

private:

    //=================================== TEST ====================
    bool test_state_flag = false;
    QTimer *testt;
    QTimer *testt_pass;
    //===================== STATE MACHINE =======================
    QStateMachine *machine;
    QState *serverSearch;                                   // поиск сети
    //===================== wicket ========================
    nikiret *wicket;
    //================== file transfer ====================
    updater _updater;

    bool ready_state_flag = false;     //  поскольку нет простого способа узнать в каком состоянии машина
    bool uncond_state_flag = false;    // сохраним пару состояний во флагах
    bool iron_mode_flag = false;
    bool test_flag = true;

    QString cmd_arg{};
    void (controller::*local_onCheck_handler)() = &controller::set_onCheckEntry; //дергается считывателем  шк
    void (controller::*remote_onCheck_handler)() = &controller::set_onCheckEXit; // дергается сервером по команде удаленного считывателя
    void set_type_Main()                          //Переключаем в режим ГЛАВНЫЙ
    {
            reader_type = _reader_type::_main;
            serverFound->set_type_Main();
            qDebug() << "Main";
    }
    void set_type_Slave()                          //Переключаем в режим ПОДЧИНЕННЫЙ
    {
            reader_type = _reader_type::slave;
            serverFound->set_type_Slave();
            qDebug() << "Slave";
    }
    void set_type_Entry()
    {
        direction_type = dir_type::entry;
        local_onCheck_handler = &controller::set_onCheckEntry; //дергается считывателем  шк
        remote_onCheck_handler = &controller::set_onCheckEXit; // дергается сервером по команде удаленного считывателя
        qDebug() << "Entry";
    }
    void set_type_Exit()
    {
        direction_type = dir_type::exit_;
        local_onCheck_handler = &controller::set_onCheckEXit; // дергается сервером по команде удаленного считывателя
        remote_onCheck_handler = &controller::set_onCheckEntry; //дергается считывателем  шк
        qDebug() << "Exit";
    }

    void set_timer()
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
    void wicket_init()
    {
        wicket = new nikiret();
        //connect(wicket, &nikiret::temp, this, &controller::send_state);

        connect(wicket, &nikiret::temp, this, [&](QString temp){emit send_to_server(message(MachineState::undef, command::getTemp, temp));});

        wicket->start();
    }
    void send_state(QString temp)
    {
        emit send_to_server(message(MachineState::undef, command::getTemp, temp));
    }

signals:
    //================= network ===========================
    void ext_provided_network_readySIG();
    void ext_provided_server_searchSIG();
    void send_to_server(message);

    void setPictureSIG(int, QString);

    void set_onCheckEntry(); //прокладка для трансляции в wicketFSM
    void set_onCheckEXit(); //прокладка для трансляции в wicketFSM

    void from_server_to_ready(); //прокладка для трансляции в wicketFSM
    void from_server_to_entry(); //прокладка для трансляции в wicketFSM
    void from_server_to_exit(); //прокладка для трансляции в wicketFSM
    void from_server_to_entryPassed(); //прокладка для трансляции в wicketFSM
    void from_server_to_exitPassed(); //прокладка для трансляции в wicketFSM
    void from_server_to_wrong(); //прокладка для трансляции в wicketFSM

    void from_server_setArmed(); //прокладка для трансляции в wicket_locker
    void from_server_setUnLocked(); //прокладка для трансляции в wicket_locker
    void log(QString);

    void exit(int);

};

#endif // CONTROLLER_H
