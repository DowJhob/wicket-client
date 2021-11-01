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
//#include <updater.h>
#include <wicketFSM/main-fsm.h>
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
    controller();
    ~controller();

    QElapsedTimer *t;
public slots:
    void start();
    void new_cmd_parse(message msg);
    void remote_barcode(QString bc);
    void local_barcode(QByteArray data);

private slots:
    void send_state2(message msg);

    //=================================== TEST MODE ====================
    void from_server_set_test();
    void from_server_set_normal();
    void timer_wrapper();
    //=================================== IRON MODE =====================
    void from_server_set_iron_mode();
    //======================================================================

private:

    //=================================== TEST ====================
    bool test_state_flag = false;
    QTimer *testt;
    QTimer *testt_pass;
    //===================== STATE MACHINE =======================
    mainFSM *machine;

    //===================== wicket ========================
    nikiret *wicket;
    //================== file transfer ====================
    //updater _updater;

    bool ready_state_flag = false;     //  поскольку нет простого способа узнать в каком состоянии машина
    bool uncond_state_flag = false;    // сохраним пару состояний во флагах
    bool iron_mode_flag = false;
    bool test_flag = true;

    QString cmd_arg{};
    void (mainFSM::*local_onCheck_handler)() = &mainFSM::set_onCheckEntry; //дергается считывателем  шк
    void (mainFSM::*remote_onCheck_handler)() = &mainFSM::set_onCheckEXit; // дергается сервером по команде удаленного считывателя

    void set_type_Main();
    void set_type_Slave();
    void set_type_Entry();
    void set_type_Exit();

    void set_timer();
    void wicket_init();
    void send_state(QString temp);

signals:
    //================= network ===========================
    void ext_provided_network_readySIG();
    void ext_provided_server_searchSIG();
    void send_to_server(message);

    void setPictureSIG(int, QString);

 //прокладка для трансляции в wicketFSM
    void from_server_to_wrong(); //прокладка для трансляции в wicketFSM

    void from_server_setArmed(); //прокладка для трансляции в wicket_locker
    void from_server_setUnLocked(); //прокладка для трансляции в wicket_locker
    void log(QString);

    void exit(int);

};

#endif // CONTROLLER_H
