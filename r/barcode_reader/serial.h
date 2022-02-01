#ifndef SERIAL_H
#define SERIAL_H

#include <QDebug>
#include <QSerialPortInfo>
#include <QSerialPort>

#include <common_types.h>

class serial: public QObject {
    Q_OBJECT
public:
    serial( bool CDC = true )
    {
        //port.setPort(portInfo);
        port.setPortName("/dev/ttyAMA0");        //  "/dev/hidraw0"
        connect(&port, &QSerialPort::readyRead, this, [this](){
            QByteArray a = port.readAll();
            qDebug() << "=========== serial::read ================" << a.toHex(':');
            //readWB();
            //if (a.size() > 0)
                //emit readyRead(a);
        });
    }
    bool open()
    {
        qDebug() << "=========== serial::open ================";
         port.setBaudRate(115200, QSerialPort::AllDirections);
         return port.open(QIODevice::ReadWrite);
//            fprintf(stdout, "can not open barcode device %s\n", port.portName().toStdString().c_str());
//  //      else
//        {
//            fprintf(stdout, "open barcode device %s\n", barcode_scaner_device.toStdString().c_str());
//        }
//        fflush(stdout);
    }

    bool close()
    {
        port.close();
        return true;
    }

signals:
    void barcode_recive();

private slots:


private:private:
    QSerialPortInfo portInfo;
    QSerialPort port;

};

#endif // SERIAL_H
