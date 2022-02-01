#ifndef SERIAL_H
#define SERIAL_H

#include <QDebug>
#include <QSerialPortInfo>
#include <QSerialPort>

#include <common_types.h>

class serial: public QObject {
    Q_OBJECT
public:
    serial( bool CDC = true );
    bool open();

    bool close();

signals:
    void barcode_recive();

private slots:


private:private:
    QSerialPortInfo portInfo;
    QSerialPort port;

};

#endif // SERIAL_H
