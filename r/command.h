#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>


#include <QVariant>
#include <QDataStream>
#include <QByteArray>
#include <QFile>
#include <QString>


enum class msg_type {
    command,
    parameter
};

enum class parameter {
    wicketArmed,
    wicketUnlocked,
    _wicketReady,
    entry_passed,
    exit_passed,
    pass_dropped,
    iron_bc, //+barcode
    barcode, //+barcode
    temp,
    wicket_state_machine_not_ready,
    remote_barcode
};

    enum class command {
    heartbeat,
    //from main to wicket
    set_test,
    set_normal,
    set_iron_mode,
    set_type_out,
    armed,
    unlock,
    onCheck,
    wrong,  //+ description
    set_ready,
    entry_open,
    exit_open,
    //from wicket to main
    _register,
    wicketArmed,
    wicketUnlocked,
    _wicketReady,
    entry_passed,
    exit_passed,
    pass_dropped,
    iron_bc, //+barcode
    barcode, //+barcode
    temp,
    wicket_state_machine_not_ready,
    remote_barcode,
    entry_barcode,
    exit_barcode,
    wrong_remote

};
/*!
  *\brief gg::ggg
  *\param gdfg
*/

typedef struct _command{
    msg_type type;
    command comm;
    QVariant body;
    _command(msg_type type = msg_type::command, command comm = command::heartbeat, QVariant body = "heartbeat"):type(type), comm(comm), body(body)
    {

    }
}message;

inline QDataStream &operator <<(QDataStream &stream,const  message &_command) // сериализуем;
{
    stream << static_cast<int>(_command.type);
    stream << static_cast<int>(_command.comm);
    stream << _command.body;
    return stream;
}

inline QDataStream &operator >>(QDataStream &stream,  message &_command) // десериализуем;
{
    int i;
    stream >> i;
    _command.type = static_cast<msg_type>(i);
    stream >> i;
    _command.comm = static_cast<command>(i);
    stream >> _command.body;
    return stream;
}


#endif // COMMAND_H
