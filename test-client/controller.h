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

private:
    //nikiret *wicket;

    //================== file transfer ====================
    //updater _updater;


    QString cmd_arg{};
//    void (mainFSM::*local_onCheck_handler)() = &mainFSM::set_onCheckEntry; //дергается считывателем  шк
//    void (mainFSM::*remote_onCheck_handler)() = &mainFSM::set_onCheckEXit; // дергается сервером по команде удаленного считывателя

    void wicket_init();

signals:
    //================= network ===========================
    void serverReady();
    void serverLost();
    void send_to_server(message);

    void setCaption(QString);

    void s_showStatus(message);

    void log(QString);

    void exit(int);

    void lamp(int);

};

#endif // CONTROLLER_H
