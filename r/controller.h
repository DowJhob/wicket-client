#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include <QProcess>
#include <QStateMachine>
#include <QSignalTransition>

#include <QThread>

#include <common_types.h>

#include <nikiret.h>
#include <updater.h>

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

public slots:
    void ext_provided_network_ready()
    {emit ext_provided_network_readySIG();}
    void ext_provided_server_search()
    {emit ext_provided_server_searchSIG();}
    void start()
    {
        wicket_init();
        set_timer();
        //==================================================================================================================
        machine = new QStateMachine(this);
        serverSearch = new QState( machine );                                   // поиск сети
        serverFound = new QState( machine );                                    // это состояние нужно для показа картинки (а может достаточно ИП) состояния на дисплее(а может и не нужно)
        //// Из любого состояния можно перейти в проебана сеть
        wicketSetArmed    = new QState( serverFound );
        wicketArmed       = new QState( serverFound );
        wicketSetUnLocked = new QState( serverFound );
        wicketUnLocked    = new QState( serverFound );
        //// Из любого состояния можно перейти в разблокированный турникет
        _wicketReady      = new QState( wicketArmed );
        _wicketOnCheck    = new QState( wicketArmed );
        _wicket_dbTimeout    = new QState( wicketArmed );
        _wicketEntry      = new QState( wicketArmed );
        _wicketPassEntry  = new QState( wicketArmed );
        _wicketExit       = new QState( wicketArmed );
        _wicketPassExit   = new QState( wicketArmed );
        _wicketWrong      = new QState( wicketArmed );
        _wicketDrop       = new QState( wicketArmed );
        _wicketError      = new QState( wicketArmed );

        machine->setInitialState( serverSearch );

        serverFound->setInitialState( wicketArmed );
        wicketArmed->setInitialState( _wicketReady );

        serverSearch->addTransition(this, SIGNAL(ext_provided_network_readySIG()), serverFound);//// Из любого состояния можно перейти в проебана сеть
        serverFound->addTransition(this, SIGNAL(ext_provided_server_searchSIG()), serverSearch);

        //это для безусловного сброса машины  по сигналу с сервера (что бы гасить лампы)
        wicketArmed->addTransition( this, SIGNAL( unconditional_set_wicketReady() ), wicketArmed );
//=========== Это для принудительной установки состояния если турникет вернет такой сигнал ============================
//======================== например при старте в разблокированном состоянии ===========================================
//============= сохраним для возможности удалить в случае если это ВЫХОДНОЙ считыватель ===============================
        wicketArmedToUnLockedTransition =
                wicketArmed->addTransition( wicket, SIGNAL( unlock() ), wicketUnLocked );
        wicketUnLockedToArmedTransition =
                wicketUnLocked->addTransition( wicket, SIGNAL( armed() ), wicketArmed );
///================================================================================
        // тут пошел нормальный цикл состояний// через сеттеры
        wicketSetUnLockedToUnLockedTransition =        //сохраним для возможности удалить в случае если это ВЫХОДНОЙ считыватель
                wicketSetUnLocked->addTransition(wicket, SIGNAL(unlock()), wicketUnLocked);    // тут ждем когда разблокируется по сигналу с кросборды
        wicketUnLocked->addTransition(this, SIGNAL(this_setArmed()), wicketSetArmed);    // тут подаем кросборде сигнал на взведение турникета
        wicketSetArmedToArmedTransition =        //сохраним для возможности удалить в случае если это ВЫХОДНОЙ считыватель
                wicketSetArmed->addTransition(wicket, SIGNAL(armed()), wicketArmed);            // тут ждем когда турникет будет готов по сигналу с кросборды
        wicketArmed->addTransition(this, SIGNAL(this_setUnLocked()), wicketSetUnLocked); // тут подаем кросборде сигнал на разблокирование турникета
//================================================================================================================================================================================
        _wicketReady->addTransition(this, SIGNAL(to_wrong()), _wicketWrong);              // по сигналу с сервера переходим в состояния запрещено
        _wicketWrong->addTransition(this, SIGNAL(to_ready()), _wicketReady);              //и обратно
        _wicketWrong->addTransition(wrong_light_timer, SIGNAL(timeout()), _wicketReady); //по сигналу таймера тоже обратно






        ///------------------------------------- собственно тут проходы ----------------------------------
        _wicketReady->addTransition(this, SIGNAL(ext_provided_barcodSIG()), _wicketOnCheck);//состояние на проверке
        _wicketReady->addTransition(this, SIGNAL(onCheck()), _wicketOnCheck);
        _wicketOnCheck->addTransition(this, SIGNAL(to_wrong()), _wicketWrong);


        _wicketOnCheck->addTransition(_db_Timeout_timer, SIGNAL(timeout()), _wicket_dbTimeout);
        _wicket_dbTimeout->addTransition(wrong_light_timer, SIGNAL(timeout()), _wicketReady);











        _wicketOnCheck->addTransition(this, SIGNAL(to_entry()), _wicketEntry);// по сигналу с сервера переходим в состояние открыто
        _wicketEntry->addTransition(wait_pass_timer, SIGNAL(timeout()), _wicketDrop);  //по сигналу таймера в состояние сброс прохода
        _wicketEntry->addTransition(wicket, SIGNAL(passed()), _wicketPassEntry);        //по сигналу прохода от турникета перейдем в состояние проход
        _wicketPassEntry->addTransition( _wicketReady );                         // и сразу в нормальное по пути отослав данные на сервер

        _wicketOnCheck->addTransition(this, SIGNAL(to_exit()), _wicketExit);
        _wicketExit->addTransition(wait_pass_timer, SIGNAL(timeout()), _wicketDrop);
        _wicketExit->addTransition(wicket, SIGNAL(passed()), _wicketPassExit);
        _wicketPassExit->addTransition( _wicketReady );

        _wicketDrop->addTransition(_wicketReady); //просто отправляет на сервер

        connect(serverSearch, &QState::entered, this, &controller::network_search); // тут покажем картинку что идет поиск (или просто включим мигание ИП
        connect(serverFound, &QState::entered, this, &controller::network_ready);   //вызовем слот  для показа картинок по сути

        //=================================================================================================
        connect(wicketUnLocked, &QState::entered, this, &controller::wicketUnLocked_slot);         // открываем или закрываем турникет
        connect(wicketArmed, &QState::entered, this, [=](){emit ext_provided_SendToServer("armed");});
        connect(wicketSetArmed, &QState::entered, this, &controller::wicketSetArmed_slot);
        connect(wicketSetUnLocked, &QState::entered, this, &controller::wicketSetUnLocked_slot);
        //=================================================================================================

        connect(_wicketReady, &QState::entered, this, &controller::set_status_READY);
        connect(_wicketReady, &QState::exited,  [=](){on_check = true;});

        connect(_wicketOnCheck, &QState::entered, this, &controller::set_status_onCheck);


        connect(_wicket_dbTimeout, &QState::entered, this, &controller::set_status_db_Timeout);






        connect(_wicketWrong, &QState::entered, this, &controller::set_status_WRONG);

        connect(_wicketEntry, &QState::entered, this, &controller::_wicketEntry_slot);  //вошли в состояния прохода, открыли калитку, зажгли лампы
        connect(_wicketExit, &QState::entered, this, &controller::_wicketExit_slot);

        connect(_wicketPassEntry, &QState::entered, this, [=](){emit ext_provided_SendToServer("entry-passed");});   // Когда входим в это состояние отсылаем сообщение на сервер
        connect(_wicketPassExit, &QState::entered, this, [=](){emit ext_provided_SendToServer("exit-passed");});

        connect(_wicketDrop, &QState::entered, this, [=](){emit ext_provided_SendToServer("pass-dropped");});

        //==================================================================================================================
        _updater.setPort( FILE_TRANSFER_PORT );
        _updater.setFilename( QCoreApplication::applicationFilePath() );
        connect(&_updater, &updater::ready, [ this ](){ emit exit(42); } );
        //==================================================================================================================

        machine->start();

    }
    void new_cmd_parse(QString msg)
    {
        QStringList cmd_list = msg.split('\n');
        //foreach ( QString sssssssssss, cmd_list )
        for ( QString sssssssssss : cmd_list )
        {
            QStringList cmd_w_value = sssssssssss.split('=');
            if (cmd_w_value.count() > 1)
                cmd_arg = cmd_w_value.at(1);
            for( int i = 0; i < 11; i++ )
                if ( cmd_w_value.at(0).indexOf(server_cmd[i].cmd_name) >= 0 )
                {
                    (this->*server_cmd[i].cmd_ptr)();
                }
        }
    }
    void send_barcode(QByteArray data)
    {
        if ( !on_check )
        {
            wicket->alarm();
            emit ext_provided_SendToServer( "barcode=" + data );
            if(iron_mode)
                (this->*handler_opener)();   //https://stackoverflow.com/questions/26331628/reference-to-non-static-member-function-must-be-called
        }
        emit ext_provided_barcodSIG();
    }
    void logger(QString s)
    {
        fprintf(stdout, "%s", s.toStdString().c_str() );
        fflush(stdout);
    }
    void set_test()
    {
        //=================================== TEST TIMER =====================================================
        testt->start(3000);
        emit ext_provided_setToTestSIG();
    }
    void set_normal()
    {
        //=================================== TEST TIMER =====================================================
        testt->stop();
        emit ext_provided_stopTestSIG();
    }
private slots:
    void network_search()
    {
        ext_provided_setPictureSIG(picture::pict_service, "");
        //        lcd_display.set_IP_toScreen( network_client->localIP + ":" + network_client->MACAddress ) ;
    }
    void network_ready()
    {
        //        _updater.setAddress(network_client->server_ip_addr);
    }
    void SendToServer(QString s)
    {
        emit ext_provided_SendToServer(s);
    }
    void network_unavailable()
    {
        ext_provided_setPictureSIG(picture::pict_service, "");
    }

    void wicketSetArmed_slot()
    {
        wicket->lock_unlock_sequence();
    }
    void wicketSetUnLocked_slot()
    {
        wicket->lock_unlock_sequence();
    }
    void wicketUnLocked_slot()
    {
        emit ext_provided_SendToServer("unlock");
        wicket->setGREEN();
        ext_provided_setPictureSIG(picture::pict_access, "");
    }

    void set_status_READY()
    {
        wrong_light_timer->stop();
        wait_pass_timer->stop();
        on_check = false;
        wicket->setLightOFF();
        ext_provided_setPictureSIG(picture::pict_ready, "");
    }
    void set_status_onCheck()
    {
        ext_provided_setPictureSIG(picture::pict_onCheck, "");
        _db_Timeout_timer->start();
    }
    void set_status_db_Timeout()
    {
        ext_provided_setPictureSIG(picture::pict_denied, "Ошибка базы данных" );
        wicket->setRED();
        wrong_light_timer->start();
    }
    void set_status_WRONG()
    {
        ext_provided_setPictureSIG(picture::pict_denied, cmd_arg );
        wicket->setRED();
        wrong_light_timer->start();
    }
    void _wicketEntry_slot()
    {
        wicket->setGREEN();
        ext_provided_setPictureSIG(picture::pict_access, "");
        wait_pass_timer->start();
        wicket->set_turnstile_to_pass(direction_state::dir_entry);
    }
    void _wicketExit_slot()
    {
        wicket->setRED();
        ext_provided_setPictureSIG(picture::pict_denied, "Подождите, вам навстречу уже кто-то идет.");
        wait_pass_timer->start();
        wicket->set_turnstile_to_pass(direction_state::dir_exit);
    }

    //    void slot_server_command_parse(QString msg)
    //    {
    //        if (msg.indexOf("sys_command") >= 0 )
    //        {
    //            QString s = msg.mid(12);
    //            QProcess::startDetached(s);
    //        }
    //        if (msg.indexOf("crsbrd_command") >= 0 )
    //        {
    //            QString s = msg.mid(15);
    //            wicket.send_to_crossboard(s.toLocal8Bit());
    //        }
    //        if (msg == "update")
    //            _updater.request_update();
    //    }

private:
    //===================== STATE MACHINE =======================
    QStateMachine *machine;
    //    QState network_not_found;                         // на случай если сети нет вообще, что бы показать это на экранчике ( тут убирать ИП или мигать им)
    QState *serverSearch;                                   // поиск сети
    QState *serverFound;                                    // это состояние нужно для показа картинки (а может достаточно ИП) состояния на дисплее(а может и не нужно)
    //// Из любого состояния можно перейти в проебана сеть
    QState *wicketSetArmed;
    QState *wicketArmed;
    QState *wicketSetUnLocked;
    QState *wicketUnLocked;
    //// Из любого состояния можно перейти в разблокированный турникет
    QState *_wicketReady;
    QState *_wicketOnCheck;
    QState *_wicket_dbTimeout;
    QState *_wicketEntry;
    QState *_wicketPassEntry;
    QState *_wicketExit;
    QState *_wicketPassExit;
    QState *_wicketWrong;
    QState *_wicketDrop;
    QState *_wicketError;

    QSignalTransition *wicketArmedToUnLockedTransition;
    QSignalTransition *wicketUnLockedToArmedTransition;

    QSignalTransition *wicketSetUnLockedToUnLockedTransition;
    QSignalTransition *wicketSetArmedToArmedTransition;
    //===================== wicket ========================
    nikiret *wicket;
    //================== file transfer ====================
    updater _updater;

    QTimer *wait_pass_timer;
    int pass_wait_time = 10000;
    QTimer *wrong_light_timer;
    int wrong_light_TIME = 2000;
    QTimer *testt;
    QTimer *_db_Timeout_timer;

    bool on_check = false;
    bool iron_mode = false;
    typedef void (controller::*MyCoolMethod)();
    struct server_cmd{
        QString cmd_name;
        MyCoolMethod cmd_ptr;
    };
    const server_cmd server_cmd[12] {
        {"set_ready", &controller::unconditional_set_wicketReady},
        {"onCheck", &controller::onCheck},
        {"set_type_out", &controller::set_type_OUT},
        {"entry-open", &controller::to_entry},               //хоть это и сигнал
        {"exit-open",&controller::to_exit},
        {"set_test", &controller::set_test},
        {"set_normal", &controller::set_normal},
        {"armed", &controller::this_setArmed},
        {"unlock", &controller::this_setUnLocked},
        {"wrong", &controller::to_wrong},
        {"iron=true", &controller::ironTRUE},
        {"iron=false", &controller::ironFALSE}
        //,{"sys_command", &controller::},
        //{"crsbrd_command", &controller::},
        //{"update", &controller::}
    };
    QString cmd_arg{};
    void (controller::*handler_opener)() = &controller::to_entry;
    void set_type_OUT()                          //Переключаем в режим на ВЫХОД
    {
        if ( main_direction != direction_state::dir_exit )
        {
            main_direction = direction_state::dir_exit;
            handler_opener = &controller::to_exit;   //https://stackoverflow.com/questions/26331628/reference-to-non-static-member-function-must-be-called
            //==================== отключим кросборду от автомата =================================
            wicketArmed->removeTransition( wicketArmedToUnLockedTransition );
            wicketUnLocked->removeTransition( wicketUnLockedToArmedTransition );

            wicketSetUnLocked->removeTransition( wicketSetUnLockedToUnLockedTransition);
            wicketSetArmed->removeTransition( wicketSetArmedToArmedTransition );

            wicketSetUnLocked->addTransition( wicketUnLocked ); //делаем их безусловными что бы серверные сигналы не переключать
            wicketSetArmed->addTransition( wicketArmed );
        }
        emit this_setArmed(); // гарантированно взводим
    }
    void set_timer()
    {
        wait_pass_timer = new QTimer(this);
        wrong_light_timer = new QTimer(this);
        testt = new QTimer(this);
        testt->setInterval(3000);
        QObject::connect(testt, &QTimer::timeout, [=](){ send_barcode("9780201379624");} );
        wait_pass_timer->setSingleShot(true);
        wrong_light_timer->setSingleShot(true);
        wait_pass_timer->setInterval(pass_wait_time);
        wrong_light_timer->setInterval(wrong_light_TIME);

        _db_Timeout_timer = new QTimer(this);
        _db_Timeout_timer->setSingleShot(true);
        _db_Timeout_timer->setInterval(wrong_light_TIME);
 //       QObject::connect(_db_Timeout_timer, &QTimer::timeout, &controller::set_status_db_Timeout );

    }
    void wicket_init()
    {
        wicket = new nikiret();
        connect(this, &controller::ext_provided_setToTestSIG, wicket, &nikiret::set_test);
        connect(this, &controller::ext_provided_stopTestSIG, wicket, &nikiret::stop_test);
        connect(wicket, SIGNAL(temp(QString)), this, SLOT(SendToServer(QString)));
        wicket->start();
    }
    //=========================
    void ironTRUE()
    {
        iron_mode = true;
    }
    void ironFALSE()
    {
        iron_mode = false;
    }
signals:
    //================= network ===========================
    void ext_provided_network_readySIG();
    void ext_provided_server_searchSIG();
    void ext_provided_SendToServer(QString);

    void ext_provided_setPictureSIG(int, QString);
    void ext_provided_barcodSIG();
    void unconditional_set_wicketReady();
    void ext_provided_setToTestSIG();
    void ext_provided_stopTestSIG();
    void exit(int);

    void to_entry();
    void to_exit();
    void to_wrong();
    void to_ready();
    void onCheck();
    void _dbTimeout();
    void this_setArmed();
    void this_setUnLocked();
    //======== TEST ==============

    void this_armed();
    void this_unlock();
};

#endif // CONTROLLER_H
