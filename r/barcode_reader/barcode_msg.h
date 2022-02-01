#ifndef BARCODE_MSG_H
#define BARCODE_MSG_H

#include <QByteArray>

class barcode_msg : public QByteArray
{
public:
    explicit barcode_msg(){}
    quint8 total_msg_count()
    {
        return this->at(1);
    }
    quint8 msg_number()
    {
        return this->at(2);
    }
    quint8 msg_size()
    {
        return this->at(3);
    }
};
#endif // BARCODE_MSG_H
