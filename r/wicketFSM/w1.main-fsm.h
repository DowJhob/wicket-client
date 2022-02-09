#ifndef MAIN_FSM_H
#define MAIN_FSM_H

#include <QStateMachine>
#include <QObject>
#include <QEventTransition>

#include <wicketFSM/w2.wicketlocker.h>

class mainFSM : public QStateMachine
{
    Q_OBJECT
public:

    mainFSM(_reader_type *reader_type,
            dir_type *direction_type,
            bool *ready_state_flag,
            bool *uncond_state_flag,
            QObject *parent);

    void set_type_Main();

    void set_type_Slave();

    void set_onCheckEntry();

    void set_onCheckEXit();

public slots:
    void fromServer(command cmd, MachineState stt);

    void fromServerState(MachineState state);

private:
    //QStateMachine m;
    bool *ready_state_flag;     //  поскольку нет простого способа узнать в каком состоянии машина
    bool *uncond_state_flag;    // сохраним пару состояний во флагах

    _reader_type *reader_type;
    dir_type *direction_type;

    //===================== STATE MACHINE =======================
    QState *serverSearch;                                   // поиск сети
    wicketLocker *serverFound;

signals:
    //================= network ===========================
    void serverFoundSIG();
    void serverSearchSIG();

    void send_to_server(message);
    void showState(showStatus);
};

#endif // MAIN_FSM_H
