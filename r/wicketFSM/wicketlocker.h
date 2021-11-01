#ifndef WICKETLOCKER_H
#define WICKETLOCKER_H

#include <QObject>
#include <QState>
#include <QSignalTransition>
#include <wicketFSM/wicketfsm.h>

class wicketLocker : public QState
{
    Q_OBJECT
public:
    wicketFSM *Armed;
    wicketLocker(nikiret *wicket,
                 _reader_type *reader_type,
                 dir_type *direction_type,
                 bool *ready_state_flag,
                 bool *uncond_state_flag,
                 QState *parent);

    void set_type_Main();

    void set_type_Slave();

    void fromServer(command cmd);

private:
    QState *UnLocked;

    QState *SetArmed;
    QState *SetUnLocked;

    bool *ready_state_flag;     //  поскольку нет простого способа узнать в каком состоянии машина
    bool *uncond_state_flag;    // сохраним пару состояний во флагах

    _reader_type *reader_type;
    dir_type *direction_type;

    nikiret *wicket;

    QSignalTransition *ArmedToUnLockedTransition;
    QSignalTransition *UnLockedToArmedTransition;

    QSignalTransition *SetUnLockedToUnLockedTransition;
    QSignalTransition *SetArmedToArmedTransition;

    void processing_Armed_();

    void processing_UnLocked();

signals:
    void from_server_setUnLocked();
    void from_server_setArmed();

    void send_to_server(message);
    void showState(picture, QString);
};

#endif // WICKETLOCKER_H
