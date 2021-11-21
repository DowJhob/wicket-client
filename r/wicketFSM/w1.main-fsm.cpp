#include "w1.main-fsm.h"

mainFSM::mainFSM(_reader_type *reader_type,
                 dir_type *direction_type,
                 bool *ready_state_flag,
                 bool *uncond_state_flag,
                 QObject *parent):
    QStateMachine(parent),
    ready_state_flag(ready_state_flag),
    uncond_state_flag(uncond_state_flag),
    reader_type(reader_type),
    direction_type(direction_type)
{
    //==================================================================================================================

    serverSearch = new QState( this );                                   // поиск сети

    serverFound = new wicketLocker(reader_type,
                                   direction_type,
                                   ready_state_flag,
                                   uncond_state_flag,
                                   this);

    connect(serverFound, &wicketLocker::showState, this, &mainFSM::showState);
    connect(serverFound, &wicketLocker::send_to_server, this, &mainFSM::send_to_server);


    serverSearch->addTransition(this, &mainFSM::serverFoundSIG, serverFound);

    serverFound->addTransition(this, &mainFSM::serverSearchSIG, serverSearch);

    connect(serverSearch, &QState::entered, this, [this](){emit showState(showStatus::Service);}); // тут покажем картинку что идет поиск (или просто включим мигание ИП

    setInitialState( serverSearch );

    //start();
    //qDebug() << "mainFSM::mainFSM";
}

void mainFSM::set_type_Main()
{
    *reader_type = _reader_type::_main;
    serverFound->set_type_Main();
}

void mainFSM::set_type_Slave()
{
    *reader_type = _reader_type::slave;
    serverFound->set_type_Slave();
}

void mainFSM::set_onCheckEntry()
{
    emit serverFound->Armed->TicketHandler->from_reader_to_onCheckEntry();
}

void mainFSM::set_onCheckEXit()
{
    emit serverFound->Armed->TicketHandler->from_reader_to_onCheckEXit();
}

void mainFSM::fromServer(command cmd, MachineState state)
{
    if (cmd != command::undef)
        switch (cmd) {
        //case command::set_type_main  : serverFound->set_type_Main();             break;
        //case command::set_type_slave : serverFound->set_type_Slave();            break;

        default                      : serverFound->fromServer(cmd) ; break;
        }
}

void mainFSM::fromServerState(MachineState state)
{
    // проксированные статусы
    serverFound->Armed->TicketHandler->fromServerState(state);
}
