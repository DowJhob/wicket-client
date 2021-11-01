#include "main-fsm.h"

mainFSM::mainFSM(nikiret *wicket,
                 _reader_type *reader_type,
                 dir_type *direction_type,
                 bool *ready_state_flag,
                 bool *uncond_state_flag,
                 QObject *parent):
    QObject(parent),
    ready_state_flag(ready_state_flag),
    uncond_state_flag(uncond_state_flag),
    reader_type(reader_type),
    direction_type(direction_type),
    wicket(wicket)
{
    //==================================================================================================================

    serverSearch = new QState( &m );                                   // поиск сети

    serverFound = new wicketLocker(wicket,
                                   reader_type,
                                   direction_type,
                                   ready_state_flag,
                                   uncond_state_flag,
                                   &m);

    serverSearch->addTransition(this, &mainFSM::serverFoundSIG, serverFound);

    serverFound->addTransition(this, &mainFSM::serverSearchSIG, serverSearch);

    connect(serverFound, &wicketLocker::send_to_server, this, &mainFSM::send_to_server);

    connect(serverSearch, &QState::entered, this, [this](){emit showState(picture::pict_service);}); // тут покажем картинку что идет поиск (или просто включим мигание ИП

    m.setInitialState( serverSearch );

    m.start();
    //qDebug() << "mainFSM::mainFSM";
}

void mainFSM::start()
{

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
    emit serverFound->Armed->set_FSM_to_onCheckEntry();
}

void mainFSM::set_onCheckEXit()
{
    emit serverFound->Armed->set_FSM_to_onCheckEXit();
}

void mainFSM::fromServer(command cmd, MachineState state)
{
    if (cmd != command::undef)
        switch (cmd) {
        case command::set_type_main  : serverFound->set_type_Main();             break;
        case command::set_type_slave : serverFound->set_type_Slave();            break;

        default                      : serverFound->fromServer(cmd) ; break;
        }
    // проксированные статусы
    if (state != MachineState::undef)
        serverFound->Armed->fromServerState(state);
}

void mainFSM::fromServerState(MachineState state)
{
    serverFound->Armed->fromServerState(state);
}
