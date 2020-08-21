#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>


#include <QVariant>
#include <QDataStream>
#include <QByteArray>
#include <QFile>
#include <QString>


enum class _type {
    command,
    parameter
};

enum class _comm {
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
    wicket_state_machine_not_ready

};

typedef struct _command{
    _type type;
    _comm comm;
    QVariant body;
    _command(_type type = _type::command, _comm comm = _comm::heartbeat, QVariant body = "heartbeat"):type(type), comm(comm), body(body)
    {

    }
}command;

inline QDataStream &operator <<(QDataStream &stream,const  command &command) // сериализуем;
{
    stream << static_cast<int>(command.type);
    stream << static_cast<int>(command.comm);
    stream << command.body;
    return stream;
}

inline QDataStream &operator >>(QDataStream &stream,  command &command) // десериализуем;
{
    int i;
    stream >> i;
    command.type = static_cast<_type>(i);
    stream >> i;
    command.comm = static_cast<_comm>(i);
    stream >> command.body;
    return stream;
}


#endif // COMMAND_H
