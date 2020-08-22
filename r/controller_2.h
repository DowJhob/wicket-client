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
        connect(wicket, &nikiret::unlock,                     serverFound, &wicketLocker::from_crsbrd_unlock);
        connect(wicket, &nikiret::armed,                      serverFound, &wicketLocker::from_crsbrd_armed);
        connect(this,   &controller::from_server_setArmed,    serverFound, &wicketLocker::from_server_setArmed);
        connect(this,   &controller::from_server_setUnLocked, serverFound, &wicketLocker::from_server_setUnLocked); // тут подаем кросборде сигнал на разблокирование турникета
        connect(this,   &controller::from_server_to_ready,    serverFound, &wicketLocker::from_crsbrd_armed);   //безусловная установка родительское состояние что бы из любого состояния получить рэди

        connect(serverFound->Armed,    &QState::entered,           this, [=](){ emit send_to_server(message(msg_type::command, command::wicketArmed)); });
        connect(serverFound->UnLocked, &QState::entered,          this, [=](){ emit send_to_server(message(msg_type::command, command::wicketUnlocked));
                                                                               setPictureSIG(picture::pict_access, "");
                                                                               wicket->setGREEN();});

        connect(serverFound->SetArmed,    &QState::entered, wicket, &nikiret::lock_unlock_sequence);
        connect(serverFound->SetUnLocked, &QState::entered, wicket, &nikiret::lock_unlock_sequence);
        ///--------------------------- Машина состояний проходов -----------------------------------------------
        connect(this,   &controller::from_server_to_wrong,  serverFound->Armed, &wicketFSM::set_FSM_to_wrong);  // по сигналу с сервера переходим в состояния запрещено
        connect(this,   &controller::set_onCheck,           serverFound->Armed, &wicketFSM::set_FSM_to_onCheck);//что бы устанавливать по команде с сервера когда ридер выходной
        connect(this,   &controller::from_server_to_entry,  serverFound->Armed, &wicketFSM::set_FSM_to_entry);  // по сигналу с сервера переходим в состояние открыто
        connect(this,   &controller::from_server_to_exit,   serverFound->Armed, &wicketFSM::set_FSM_to_exit);
        connect(wicket, &nikiret::passed,                   serverFound->Armed, &wicketFSM::set_FSM_passed);   //по сигналу прохода от турникета перейдем в состояние проход

        connect(serverFound->Armed->Ready,         &QState::entered, this, [=](){ready_state_flag = true;
            wicket->setLightOFF();
            setPictureSIG(picture::pict_ready, "");
         emit send_to_server(message(msg_type::command, command::_wicketReady));});
        connect(serverFound->Armed->Ready,         &QState::exited,  this, [=](){ready_state_flag = false;});
        connect(serverFound->Armed->OnCheck,       &QState::entered, this, [=](){setPictureSIG(picture::pict_onCheck, "");});
        connect(serverFound->Armed->dbTimeout,     &QState::entered, this, [=](){setPictureSIG(picture::pict_denied, "Ошибка базы данных" ); wicket->setRED(); });
        connect(serverFound->Armed->Wrong,         &QState::entered, this, [=](){setPictureSIG(picture::pict_denied, cmd_arg ); wicket->setRED();
                                                                                 qDebug() << "time to enterd WRONG state: " << cmd_arg << t->nsecsElapsed()/1000000 << "ms";});
        connect(serverFound->Armed->Entry,         &QState::entered, this, &controller::_wicketEntry_slot);  //вошли в состояния прохода, открыли калитку, зажгли лампы
        connect(serverFound->Armed->Exit,          &QState::entered, this, &controller::_wicketExit_slot);
        connect(serverFound->Armed->entryPassed,   &QState::entered, this, [=](){ emit send_to_server(message(msg_type::command, command::entry_passed));} );   // Когда входим в это состояние отсылаем сообщение на сервер
        connect(serverFound->Armed->exitPassed,    &QState::entered, this, [=](){ emit send_to_server(message(msg_type::command, command::entry_passed));} );
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
              case command::set_test :     from_server_set_test();break;
              case command::set_normal:    from_server_set_normal();break;
              case command::set_iron_mode: from_server_set_iron_mode();break;
              case command::set_type_out:  from_server_set_type_OUT();break;
              case command::armed:         from_server_setArmed();break;
              case command::unlock:        from_server_setUnLocked();break;
              case command::onCheck:       from_server_onCheck();break;
              case command::wrong:         qDebug()<<"time betwen send barcode and recieve wrong ========================================================== WRONG: "
                                                << QString::number(t->nsecsElapsed()/1000000) << "ms";
                                                 t->restart();
                                                 from_server_to_wrong ();break;//+ description
              case command::set_ready:     from_server_to_ready();break;
              case command::entry_open:    qDebug()<<"time betwen send barcode and recieve open ========================================================== OPEN: "
                                                << QString::number(t->nsecsElapsed()/1000000) << "ms";
                                                 t->restart();
                                                  from_server_to_entry();break;
              case command::exit_open:     from_server_to_exit();break;
              }


    }
    void send_barcode(QByteArray data)
    {
        t->start();
        if ( ready_state_flag )
        {
            emit set_onCheck();
            if (!test_state_flag)
                wicket->alarm();

            if(iron_mode_flag)
            {
                (this->*handler_opener)();   //https://stackoverflow.com/questions/26331628/reference-to-non-static-member-function-must-be-called
                emit send_to_server(message(msg_type::command, command::iron_bc, data ));
            }
            else
                emit send_to_server(message(msg_type::command, command::barcode, data ));
        }


    }

private slots:
    //=================================== TEST MODE ====================
    void from_server_set_test()
    {
        //=================================== TEST TIMER =====================================================
        qDebug() << "set test ==============================================================================TEST";
        test_state_flag = true;
        testt->start(3000);
        connect(testt_pass, &QTimer::timeout,      serverFound->Armed, &wicketFSM::set_FSM_passed);
        connect(serverFound->Armed->Entry, &QState::entered,   this, &controller::timer_wrapper);
        connect(serverFound->Armed->Exit,  &QState::entered,   this, &controller::timer_wrapper);
    }
    void from_server_set_normal()
    {
        //=================================== TEST TIMER =====================================================
        qDebug() << "set normal==============================================================================NORMAL";
        test_state_flag = false;
        iron_mode_flag = false;
        testt->stop();
        testt_pass->stop();
        disconnect(testt_pass, &QTimer::timeout,      serverFound->Armed, &wicketFSM::set_FSM_passed);
        disconnect(serverFound->Armed->Entry, &QState::entered,   this, &controller::timer_wrapper);
    }
    void timer_wrapper()
    {
        qDebug()<<"wrapper";
        testt_pass->start();
    }
    //========================= IRON MODE ===============================
    void from_server_set_iron_mode()
    {
        qDebug() << "set iron ==============================================================================IRON";
        iron_mode_flag = true;
    }
    void from_server_reset_iron_mode()
    {
        iron_mode_flag = false;
    }
    //======================================================================
    void _wicketEntry_slot()
    {
        wicket->setGREEN();
        qDebug() << "time to enterd ENTRY state: " << cmd_arg << t->nsecsElapsed()/1000000 << "ms";
        setPictureSIG(picture::pict_access, "");
        wicket->set_turnstile_to_pass(direction_state::dir_entry);
    }
    void _wicketExit_slot()
    {
        wicket->setRED();
        setPictureSIG(picture::pict_denied, "Подождите, вам навстречу уже кто-то идет.");
        qDebug() << "time to enterd EXIT entry state: " << cmd_arg << t->nsecsElapsed()/1000000 << "ms";
        wicket->set_turnstile_to_pass(direction_state::dir_exit);
    }

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
    struct server_cmd{
        QString cmd_name;
        MyCoolMethod cmd_ptr;
    };

    QString cmd_arg{};
    void (controller::*handler_opener)() = &controller::from_server_to_entry;
    void from_server_set_type_OUT()                          //Переключаем в режим на ВЫХОД
    {
        if ( main_direction != direction_state::dir_exit )
        {
            main_direction = direction_state::dir_exit;
            handler_opener = &controller::from_server_to_exit;   //https://stackoverflow.com/questions/26331628/reference-to-non-static-member-function-must-be-called
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
        connect(testt, &QTimer::timeout, [=](){ send_barcode("9780201379624");} );
    }
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
    void set_onCheck();

    void exit(int);

    void from_server_to_ready();
    void from_server_to_entry();
    void from_server_to_exit();
    void from_server_to_wrong();
    void from_server_onCheck();
    void from_server_setArmed();
    void from_server_setUnLocked();
    void log(QString);

};

#endif // CONTROLLER_H
