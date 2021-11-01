#ifndef WICKETFSM_H
#define WICKETFSM_H

#include <QDebug>
#include <QSignalTransition>
//#include <QAbstractTransition>
#include <QObject>
#include <QState>
#include <QTimer>

#include "command.h"
#include <nikiret.h>

class wicketFSM : public QState
{
    Q_OBJECT
public:
    wicketFSM(nikiret *wicket,
              _reader_type *reader_type,
              dir_type *direction_type,
              bool *ready_state_flag,
              bool *uncond_state_flag,
              QState *parent);

    void set_type_Slave();

    void set_type_Main();

public slots:
    void set_uncondDelay_time(int uncondDelay = 6000);

    void fromServer(command cmd);
    void fromServerState(MachineState state);

private:
    QState *Ready;
    QState *OnCheckEntry;
    QState *OnCheckEXit;
    QState *dbTimeout;
    QState *Entry;
    QState *entryPassed;
    QState *Exit;
    QState *exitPassed;
    QState *Wrong;
    QState *Drop;
    QState *UncondTimeout;


    bool *ready_state_flag;     //  поскольку нет простого способа узнать в каком состоянии машина
    bool *uncond_state_flag;    // сохраним пару состояний во флагах

    _reader_type *reader_type;
    dir_type *direction_type;

    nikiret *wicket;

    QString cmd_arg{};

    QSignalTransition *unCondToReady = nullptr;
    QSignalTransition *unCondToCheckExit = nullptr;
    QAbstractTransition *unCondToReadySlaveType = nullptr;

    QTimer *wait_remote_timer;
    QTimer *wait_pass_timer;
    QTimer *wrong_light_timer;
    QTimer *_db_Timeout_timer;
    QTimer *uncondDelayTimer;
    int wait_remote_time = 1000;
    int pass_wait_time = 10000;
    int wrong_light_TIME = 2000;
    //    int uncondDelay = 8000;

    void setActions();

    void set_timer();

    void set_onCheckEntry();

    void set_onCheckEXit();

private slots:
    void send_state2(message msg);

    void processing_dbTimeout();

    void processing_Wrong();

    void processing_Ready();

    void processing_exReady();

    void sub_processing_onCheck(MachineState state);

    void processing_onCheckEntry();

    void processing_onCheckExit();

    void processing_Entry();

    void processing_Exit();

    void processing_EntryPassed();

    void processing_ExitPassed();

    void processing_UncondTimeout();

    void processing_exUncondTimeout();

    void processing_Drop();

signals:
    void from_server_to_entry();
    void from_server_to_exit();
    void from_server_to_entryPassed();
    void from_server_to_exitPassed();
    void from_server_to_wrong();


    void set_FSM_passed();


    void set_FSM_to_onCheckEntry();
    void set_FSM_to_onCheckEXit();

    void send_to_server(message);
    void showState(picture, QString);

};

#endif // WICKETFSM_H
