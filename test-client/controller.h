#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QDebug>
//#include <QProcess>

#include <QTime>
#include <QTimer>

#include <QThread>
#include <QElapsedTimer>

#include "../common/common_types.h"
#include "../common/command.h"
//#include <nikiret.h>
//#include <updater.h>

#define CROSSBOARD_SERIAL_PORT_NAME     "/dev/ttyAPP3"
#define FILE_TRANSFER_PORT              27008

class controller: public QObject
{
    Q_OBJECT
public:
    _reader_type reader_type = _reader_type::_main;
    dir_type direction_type = dir_type::entry;
    controller();

    QElapsedTimer *t;

public slots:
    void start();
    void new_cmd_parse(message msg);

    void local_barcode(QByteArray data);

private slots:
    //=================================== TEST MODE ====================
    void from_server_set_test();
    void from_server_set_normal();
    void timer_wrapper();
    //=================================== IRON MODE =====================
    void from_server_set_iron_mode();
    //======================================================================

private:
    //nikiret *wicket;
    //=================================== TEST ====================
    bool test_state_flag = false;
    QTimer *testt;
    QTimer *testt_pass;

    //================== file transfer ====================
    //updater _updater;

    bool iron_mode_flag = false;
    bool test_flag = true;

    QString cmd_arg{};
//    void (mainFSM::*local_onCheck_handler)() = &mainFSM::set_onCheckEntry; //дергается считывателем  шк
//    void (mainFSM::*remote_onCheck_handler)() = &mainFSM::set_onCheckEXit; // дергается сервером по команде удаленного считывателя

    void set_timer();
    void wicket_init();

signals:
    //================= network ===========================
    void serverReady();
    void serverLost();
    void send_to_server(message);

    void s_showStatus(message);

 //прокладка для трансляции в wicketFSM
//    void from_server_to_wrong(); //прокладка для трансляции в wicketFSM

//    void from_server_setArmed(); //прокладка для трансляции в wicket_locker
//    void from_server_setUnLocked(); //прокладка для трансляции в wicket_locker
    void log(QString);

    void exit(int);

};

#endif // CONTROLLER_H
