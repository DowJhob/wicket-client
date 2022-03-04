#ifndef SERIAL_H
#define SERIAL_H

#include "ibarcode.h"

#include <QDebug>
#include <QSerialPortInfo>
#include <QSerialPort>

typedef struct
{
    quint8 retransmit   : 1;
    quint8 continuation : 1;
    quint8 reserved     : 1;
    quint8 changeType   : 1;
    quint8 unused       : 4;
} Status;

typedef struct
{
    quint8 length;
    quint8 opcode;
    quint8 messageSource;
    Status status;
    quint8 barcodeType;
} headerSSI;

class serial: public IBarcode
{
    Q_OBJECT
public:
    serial();
    void open();

    bool close();

private slots:
    QSerialPortInfo getScaner();
    void packetSlicer();

private:
    QSerialPortInfo portInfo;
    QSerialPort port;

    QByteArray decodedData{};

    void getCheckSum(QByteArray com)
    {

    }

signals:

};

#endif // SERIAL_H
