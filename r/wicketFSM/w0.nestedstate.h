#ifndef W0_H
#define W0_H

#include <QStateMachine>
#include <QState>

#include <common_types.h>
#include "command.h"

class nestedState : public QState
{
    Q_OBJECT
public:
    nestedState(nestedState *parent):QState(parent)
    {
    }
    nestedState(QStateMachine *parent):QState(parent)
    {
    }

public slots:
    virtual void fromServer(command cmd, MachineState stt) = 0;
    virtual void fromServerState(MachineState state) = 0;

signals:
    void from_server_to_entry();
    void from_server_to_exit();
    void from_server_to_entryPassed();
    void from_server_to_exitPassed();
    void from_server_to_wrong();

    void send_to_server(message);
    void showState(showStatus);
};

#endif // W0_H
