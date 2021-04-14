#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>


#include <QVariant>
#include <QDataStream>
#include <QByteArray>
#include <QFile>
#include <QString>

enum class MachineState {
undef,
    onArmed,
    onUnlocked,

    getRemoteBarcode,
    onWrongRemote,

    onReady,
    onWrong,
    onCheckEntry,
    onCheckExit,
    onEntry,
    onExit,
    onEtryPassed,
    onExitPassed,
    onPassDropped,
    onUncodTimeout,
    on_dbTimeout,

    onStateMachineNotReady,
};
enum class command {
    undef,
    heartbeat,
    //from main to wicket
    set_test,
    set_normal,
    set_iron_mode,
    set_type_out,
    set_Armed,
    set_Unlock,

    set_Wrong,  //+ description
    set_Ready,
    set_EntryOpen,
    set_ExitOpen,
    //from wicket to main
    get_Register,

    getIronBC, //+barcode
    getBarcode, //+barcode
    getTemp

};
/*!
  *\brief gg::ggg
  *\param gdfg
*/

typedef struct _message{
    MachineState state;
    command cmd;
    QVariant body;
    _message( MachineState state = MachineState::undef, command comm = command::heartbeat, QVariant body = "heartbeat")
        : state(state), cmd(comm), body(body)
    {

    }/*
    _message( command comm = command::heartbeat, QVariant body = "heartbeat"):
        state(MachineState::undef), cmd(comm), body(body)
    {

    }
    _message( MachineState state = MachineState::undef, QVariant body = "heartbeat"):
        state(state), cmd(command::heartbeat), body(body)
    {

    }
    _message( MachineState state = MachineState::undef):
        state(state), cmd(command::heartbeat), body("")
    {

    }
    _message(): state(MachineState::undef), cmd(command::heartbeat), body("")
    {

    }*/
}message;

inline QDataStream &operator <<(QDataStream &stream,const  message &msg) // сериализуем;
{
    stream << static_cast<int>(msg.state);
    stream << static_cast<int>(msg.cmd);
    stream << msg.body;
    return stream;
}

inline QDataStream &operator >>(QDataStream &stream,  message &msg) // десериализуем;
{
    int i;
    stream >> i;
    msg.state = static_cast<MachineState>(i);
    stream >> i;
    msg.cmd = static_cast<command>(i);
    stream >> msg.body;
    return stream;
}



#endif // COMMAND_H
