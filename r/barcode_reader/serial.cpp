#include "serial.h"

serial::serial()
{
    open();
    port.flush();
    connect(&port, &QSerialPort::readyRead, this, &serial::packetSlicer);

    //    connect(&port, &QSerialPort::breakEnabledChanged, this, [](bool set){
    //        qDebug() << "=========== serial::breakEnabledChanged =" << set;
    //    });
    //    connect(&port, &QSerialPort::dataTerminalReadyChanged, this, [](bool set){
    //        qDebug() << "=========== serial::dataTerminalReadyChanged =" << set;
    //    });
    //    connect(&port, &QSerialPort::requestToSendChanged, this, [](bool set){
    //        qDebug() << "=========== serial::requestToSendChanged =" << set;
    //    });

}

bool serial::open()
{
    qDebug() << "=========== serial::open ================";
    //port.setPort(portInfo);
    port.setPortName("COM5");        //  "/dev/hidraw0"
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

void serial::packetSlicer()
{
    while(port.bytesAvailable() > 7)  //  header + хотя бы один байт данных и чексум
    {
        QByteArray buffer = port.read(5);

        headerSSI *header = reinterpret_cast<headerSSI*>(buffer.data());
        int count = header->length - 3;
        if(port.bytesAvailable() >= count)
        {
            decodedData.append(port.read(count));
            decodedData.chop(2);  // отрежем чексум
        }
        if(!header->status.continuation)
        {
            emit decoded(decodedData);
            decodedData.clear();
        }
    }
}
