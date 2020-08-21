#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>


#include <QtCore/QCoreApplication>
#include <QDataStream>
#include <QByteArray>
#include <QFile>
#include <QString>
#include <QHash>
#include <QDebug>




enum class _type {
    command,
    parameter
};

typedef struct _command{
    _type type;
    QVariant body;
    _command(_type type = _type::command, QVariant body = "heartbeat"):type(type), body(body)
    {

    }
}command;

inline QDataStream &operator <<(QDataStream &stream,const  command &command) // сериализуем;
{
    stream << static_cast<int>(command.type);
    //   stream << sC.b;
    stream << command.body;
    return stream;
}

inline QDataStream &operator >>(QDataStream &stream,  command &command) // десериализуем;
{
    int i;
    stream >> i;
    command.type = static_cast<_type>(i);
    stream >> command.body;
    //    stream >> sC.c;
    return stream;
}


#endif // COMMAND_H
