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
    QString ticket_status_description;
    QString value_cmd;
    int main_direction = direction_state::dir_entry;
    controller()
    { }
    wicketLocker *serverFound;
    QElapsedTimer *t;
public slots:
    void ext_provided_network_ready()
    {emit ext_provided_network_readySIG();}
    void ext_provided_server_search()
    {emit ext_provided_server_searchSIG();}
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

        serverSearch->addTransition(this, SIGNAL(ext_provided_network_readySIG()), serverFound);//// Из любого состояния можно перейти в проебана сеть
        serverFound->addTransition(this, SIGNAL(ext_provided_server_searchSIG()), serverSearch);//// Из любого состояния можно перейти в проебана сеть

        connect(serverSearch, &QState::entered, this, [=](){setPictureSIG(picture::pict_service, "");}); // тут покажем картинку что идет поиск (или просто включим мигание ИП
        //   connect(_wicketLocker_serverFound, &QState::entered, this, &controller::set_status_READY);   //вызовем слот  для показа картинок по сути

        machine->setInitialState( serverSearch );
        ///--------------------------- Машина сброса-взвода прохода -----------------------------------------------
        //        connect(this,   &controller::from_server_setReady,    _wicketLocker_serverFound, &wicketLocker::in_uncond);
        connect(wicket, &nikiret::unlocked,                     serverFound, &wicketLocker::from_crsbrd_unlock);
        connect(wicket, &nikiret::armed,                      serverFound, &wicketLocker::from_crsbrd_armed);
        connect(this,   &controller::from_server_setArmed,    serverFound, &wicketLocker::from_server_setArmed);
        connect(this,   &controller::from_server_setUnLocked, serverFound, &wicketLocker::from_server_setUnLocked); // тут подаем кросборде сигнал на разблокирование турникета
        connect(this,   &controller::from_server_to_ready,    serverFound, &wicketLocker::from_server_setArmed);   //безусловная установка родительское состояние что бы из любого состояния получить рэди

        connect(serverFound->Armed,    &QState::entered,           this, [=](){ emit send_to_server(message(msg_type::command, command::wicketArmed)); });
        connect(serverFound->UnLocked, &QState::entered,          this, [=](){ emit send_to_server(message(msg_type::command, command::wicketUnlocked));
            setPictureSIG(picture::pict_access, "");
            wicket->setGREEN();});

        connect(serverFound->SetArmed,    &QState::entered, wicket, &nikiret::lock_unlock_sequence);
        connect(serverFound->SetUnLocked, &QState::entered, wicket, &nikiret::lock_unlock_sequence);
        ///--------------------------- Машина состояний проходов -----------------------------------------------
        connect(this,   &controller::from_server_to_wrong,  serverFound->Armed, &wicketFSM::set_FSM_to_wrong);  // по сигналу с сервера переходим в состояния запрещено
        connect(this,   &controller::from_server_to_entry,  serverFound->Armed, &wicketFSM::set_FSM_to_entry);  // по сигналу с сервера переходим в состояние открыто
        connect(this,   &controller::from_server_to_exit,   serverFound->Armed, &wicketFSM::set_FSM_to_exit);
        connect(wicket, &nikiret::passed,                   serverFound->Armed, &wicketFSM::set_FSM_passed);   //по сигналу прохода от турникета перейдем в состояние проход

        connect(this,   &controller::set_onCheckEntry,      serverFound->Armed, &wicketFSM::set_FSM_to_onCheckEntry);
        connect(this,   &controller::set_onCheckEXit,       serverFound->Armed, &wicketFSM::set_FSM_to_onCheckEXit);

        //        connect(this,   &controller::from_server_onCheckEntry, serverFound->Armed, &wicketFSM::set_FSM_to_onCheckEntry);
        //       connect(this,   &controller::from_server_onCheckEXit, serverFound->Armed, &wicketFSM::set_FSM_to_onCheckEXit);

        //============= processing ===================================
        connect(serverFound->Armed->Ready,         &QState::entered, this, &controller::processing_Armed);
        connect(serverFound->Armed->Ready,         &QState::exited,  this, [=](){ready_state_flag = false;});
        connect(serverFound->Armed->OnCheckEntry,  &QState::entered, this, &controller::processing_onCheckEntry);
        connect(serverFound->Armed->OnCheckEXit,   &QState::entered, this, &controller::processing_onCheckExit);
        connect(serverFound->Armed->dbTimeout,     &QState::entered, this, [=](){setPictureSIG(picture::pict_denied, "Ошибка базы данных" ); wicket->setRED(); });
        connect(serverFound->Armed->Wrong,         &QState::entered, this, [=](){setPictureSIG(picture::pict_denied, cmd_arg ); wicket->setRED();});
        connect(serverFound->Armed->Entry,         &QState::entered, this, &controller::processing_wicketEntry_slot);  //вошли в состояния прохода, открыли калитку, зажгли лампы
        connect(serverFound->Armed->Exit,          &QState::entered, this, &controller::processing_wicketExit_slot);
        connect(serverFound->Armed->entryPassed,   &QState::entered, this, [=](){ emit send_to_server(message(msg_type::command, command::entry_passed));} );   // Когда входим в это состояние отсылаем сообщение на сервер
        connect(serverFound->Armed->exitPassed,    &QState::entered, this, [=](){ emit send_to_server(message(msg_type::command, command::exit_passed));} );
        connect(serverFound->Armed->Drop,          &QState::entered, this, [=](){ emit send_to_server(message(msg_type::command, command::pass_dropped));} );

        connect(serverFound->Armed->UncondTimeout, &QState::entered, this, [=](){ setPictureSIG(picture::pict_timeout, "");} );
        ///-----------------------------------------------------
        //==================================================================================================================
        _updater.setPort( FILE_TRANSFER_PORT );
        _updater.setFilename( QCoreApplication::applicationFilePath() );
        connect(&_updater, &updater::ready, [ this ](){ emit exit(42); } );
        //==================================================================================================================
        machine->start();
    }
    void new_cmd_parse(message msg)
    {
        cmd_arg = msg.body.toString();
        switch (msg.comm) {
        //     case _comm::heartbeat: ;
        //from main to wicket
        case command::set_test :      emit from_server_set_test();       break;
        case command::set_normal:     emit from_server_set_normal();     break;
        case command::set_iron_mode:  emit from_server_set_iron_mode();  break;
        case command::set_type_out:   emit from_server_set_type_OUT();   break;
        case command::armed:          emit from_server_setArmed();       break;
        case command::unlock:         emit from_server_setUnLocked();    break;
            //        case command::onCheck:        (this->*remote_onCheck_handler)(); break;
        case command::set_ready:      emit from_server_to_ready();       break;
        case command::wrong:          emit from_server_to_wrong ();      break;
        case command::entry_open:     emit from_server_to_entry();       break;
        case command::exit_open:      emit from_server_to_exit();        break;

        case command::remote_barcode: remote_barcode(cmd_arg);           break;
        case command::wrong_remote:   emit from_server_to_wrong ();      break;
        default: break;
        }
    }
    void remote_barcode(QString bc)
    {
        //if ( machine->configuration().contains(serverFound->Armed->Ready))
        if ( ready_state_flag )
        {
            (this->*remote_onCheck_handler)();
            //send_to_server(message(msg_type::command, command::exit_barcode, bc ));
        }
        else
            send_to_server(message(msg_type::command, command::wrong_remote ));
    }
    void local_barcode(QByteArray data)
    {
        cmd_arg = data;
        t->start();
        if ( ready_state_flag )
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
    void processing_Armed()
    {
        ready_state_flag = true;
        wicket->setLightOFF();
        setPictureSIG(picture::pict_ready, "");
        emit send_to_server(message(msg_type::command, command::_wicketReady));
    }
    void processing_onCheckEntry()
    {
        //(this->*handler_opener)();
        //emit send_to_server(message(msg_type::command, command::iron_bc, "" ));
        if(main_direction == dir_entry)
            emit send_to_server(message(msg_type::command, command::entry_barcode, cmd_arg ));
        else if(main_direction == dir_exit)
        {
            //nothing to do
        }
        setPictureSIG(picture::pict_onCheck, "");
    }
    void processing_onCheckExit()
    {
        if(main_direction == dir_entry)
            emit send_to_server(message(msg_type::command, command::exit_barcode, cmd_arg ));
        else if(main_direction == dir_exit)
            emit send_to_server(message(msg_type::command, command::remote_barcode, cmd_arg ));
        setPictureSIG(picture::pict_onCheck, "");
    }
    void processing_wicketEntry_slot()
    {
        if(main_direction == dir_entry)
        {
            wicket->setGREEN();
            setPictureSIG(picture::pict_access, "");
        }
        else if(main_direction == dir_exit)
        {
            wicket->setRED();
            setPictureSIG(picture::pict_denied, "Подождите, вам навстречу уже кто-то идет.");
        }
        wicket->set_turnstile_to_pass(direction_state::dir_entry);
    }
    void processing_wicketExit_slot()
    {
        if(main_direction == dir_entry)
        {
            wicket->setRED();
            setPictureSIG(picture::pict_denied, "Подождите, вам навстречу уже кто-то идет.");
        }
        else if(main_direction == dir_exit)
        {
            wicket->setGREEN();
            //qDebug() << "time to enterd ENTRY state: " << cmd_arg << t->nsecsElapsed()/1000000 << "ms";
            setPictureSIG(picture::pict_access, "");
        }
        wicket->set_turnstile_to_pass(direction_state::dir_exit);
    }
    //=================================== TEST MODE ====================
    void from_server_set_test()
    {
        //=================================== TEST TIMER =====================================================
        qDebug() << "set test ==============================================================================TEST";
        test_state_flag = true;
        testt->start(3000);

        if(main_direction == direction_state::dir_entry )
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

    bool ready_state_flag = true;
    bool iron_mode_flag = false;
    typedef void (controller::*MyCoolMethod)();
    typedef void (controller::*onCheckType)();
    struct server_cmd{
        QString cmd_name;
        MyCoolMethod cmd_ptr;
    };

    QString cmd_arg{};
    void (controller::*handler_opener)() = &controller::from_server_to_entry;
    void (controller::*local_onCheck_handler)() = &controller::set_onCheckEntry; //дергается считывателем  шк
    void (controller::*remote_onCheck_handler)() = &controller::set_onCheckEXit; // дергается сервером по команде удаленного считывателя
    void from_server_set_type_OUT()                          //Переключаем в режим на ВЫХОД
    {
        if ( main_direction != direction_state::dir_exit )
        {
            main_direction = direction_state::dir_exit;
            handler_opener = &controller::from_server_to_exit;   //https://stackoverflow.com/questions/26331628/reference-to-non-static-member-function-must-be-called

            local_onCheck_handler = &controller::set_onCheckEXit;
            remote_onCheck_handler = &controller::set_onCheckEntry;
            //==================== отключим кросборду от автомата =================================
            serverFound->set_type_OUT();
        }
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
    bool test_flag = true;
    void wicket_init()
    {
        wicket = new nikiret();
        connect(wicket, &nikiret::temp, this, &controller::send_state);
        wicket->start();
    }
    void send_state(QString temp)
    {
        emit send_to_server(message(msg_type::command, command::temp, temp));
        if ( machine->isRunning() )
        {
            //qDebug() << machine->configuration();
            if (machine->configuration().contains( serverFound->Armed->Ready))
                emit send_to_server(message(msg_type::command, command::_wicketReady));
            if ( machine->configuration().contains( serverFound->Armed ) )
                emit send_to_server(message(msg_type::command, command::wicketArmed));
            if ( machine->configuration().contains( serverFound->UnLocked ) )
                emit send_to_server(message(msg_type::command, command::wicketUnlocked));
        }
        else
            emit send_to_server(message(msg_type::command, command::wicket_state_machine_not_ready));

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
    void from_server_to_wrong(); //прокладка для трансляции в wicketFSM
    //    void from_server_onCheck();
    void from_server_setArmed(); //прокладка для трансляции в wicket_locker
    void from_server_setUnLocked(); //прокладка для трансляции в wicket_locker
    void log(QString);

    void exit(int);

};

#endif // CONTROLLER_H
