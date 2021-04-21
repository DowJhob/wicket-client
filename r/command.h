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

    //from main to wicket
    set_test,
    set_normal,
    set_iron_mode,

    set_type_main,
    set_type_slave,

    set_type_entry,   // Направление пропуска данного считывателя
    set_type_exit,

    set_Armed,
    set_Unlock,

    set_Wrong,  //+ description
    set_Ready,
    set_EntryOpen,
    set_ExitOpen,

    //from wicket to main
    get_Register,
    getBarcode, //+barcode
    getTemp,

    heartbeat
};
/*!
  *\brief gg::ggg
  *\param gdfg
*/

typedef struct message{
    MachineState state;
    command cmd;
    QVariant body;
//    int param;
    message( MachineState state = MachineState::undef, command cmd = command::heartbeat, QVariant body = "heartbeat"):
            state(state), cmd(cmd), body(body)
        {}
//    message( command cmd = command::heartbeat ):
//            state(MachineState::undef), cmd(cmd), body("")
//        {}
//    message( MachineState state = MachineState::undef):
//            state(state), cmd(command::undef), body("")
//        {}
//    explicit message( ):
//            state(MachineState::undef), cmd(command::heartbeat), body("heartbeat")
//        {}
/*

    explicit message(): body("heartbeat"), param(0)
    {}
    explicit message( MachineState state, QVariant body): body(body), param(static_cast<int>(state) )
    {}
    explicit message( command cmd, QVariant body):body(body), param(static_cast<int>(cmd))
    {}
*/
}mmessage;

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
