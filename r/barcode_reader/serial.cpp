#include "serial.h"

serial::serial(bool CDC)
{
    open();
    connect(&port, &QSerialPort::readyRead, this, [this](){
        QByteArray a = port.readAll();
        qDebug() << "=========== serial::read =" << a.toHex(':');
        qDebug() << "=========== serial::read =" << a;
        port.write(QByteArray("\xD0"));
        //readWB();
        //if (a.size() > 0)
        //emit readyRead(a);
    });

    connect(&port, &QSerialPort::breakEnabledChanged, this, [](bool set){
        qDebug() << "=========== serial::breakEnabledChanged =" << set;
    });
    connect(&port, &QSerialPort::dataTerminalReadyChanged, this, [](bool set){
        qDebug() << "=========== serial::dataTerminalReadyChanged =" << set;
    });
    connect(&port, &QSerialPort::requestToSendChanged, this, [](bool set){
        qDebug() << "=========== serial::requestToSendChanged =" << set;
    });

}

bool serial::open()
{
    qDebug() << "=========== serial::open ================";
    //port.setPort(portInfo);
    port.setPortName("/dev/ttyACM0");        //  "/dev/hidraw0"
    port.setBaudRate(115200, QSerialPort::AllDirections);
    return port.open(QIODevice::ReadWrite);
    //            fprintf(stdout, "can not open barcode device %s\n", port.portName().toStdString().c_str());
    //  //      else
    //        {
    //            fprintf(stdout, "open barcode device %s\n", barcode_scaner_device.toStdString().c_str());
    //        }
    //        fflush(stdout);
}

bool serial::close()
{
    port.close();
    return true;
}
