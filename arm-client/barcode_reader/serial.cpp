#include "serial.h"

serial::serial()
{
open();
}

void serial::open()
{
    //QString portName = getScaner();
    QSerialPortInfo portInfo = getScaner();
    //if(portName.isEmpty())
    //    return false;
    //qDebug() << "=========== serial::open ================";
    port.setPort(portInfo);
    //port.setPortName(portName);        //  "/dev/hidraw0"
    port.setBaudRate(115200, QSerialPort::AllDirections);
    bool o = port.open(QIODevice::ReadWrite);



    if(o)
    {
        //QSerialPortInfo spp(port);
        qDebug() << "== serial::open" << port.portName() << portInfo.description() << portInfo.productIdentifier()
                 << portInfo.vendorIdentifier() << "::opened";
        port.flush();
        connect(&port, &QSerialPort::readyRead, this, &serial::packetSlicer);
        connect(&port, &QSerialPort::errorOccurred, this, [](QSerialPort::SerialPortError error)
        {qDebug() << "=========== serial::errorOccurred" << error;});
        connect(&port, &QSerialPort::aboutToClose, this, [](){qDebug() << "=========== serial::aboutToClose";});
        connect(&port, &QSerialPort::readChannelFinished, this, [](){qDebug() << "=========== serial::readChannelFinished";});
qDebug() << "== serial::::";
    }
    else
        qDebug() << "== serial::" << portInfo.portName() << ":dont open";


}

bool serial::close()
{
    port.close();
    return true;
}

QSerialPortInfo serial::getScaner()
{
    auto l = QSerialPortInfo::availablePorts();
    //qDebug() << "=========== serial::getScaner ================" << l.size();
    for(QSerialPortInfo sp : l)
    {
        qDebug() << "== serial::getScaner" << sp.portName() << sp.description() << sp.vendorIdentifier() << sp.productIdentifier();
        if (sp.productIdentifier() == 0x1701
                && sp.vendorIdentifier() == 0x05E0)
        {
            //qDebug() << "=========== serial::getScaner::portName()" << sp.portName();
            return sp;
        }
    }

}

void serial::packetSlicer()
{
    qDebug() << "=========== serial::packetSlicer bytesAvailable" << port.bytesAvailable();
    while(port.bytesAvailable() > 7)  //  header + хотя бы один байт данных и чексум
    {
        QByteArray buffer = port.read(5);
        qDebug() << "=========== serial::packetSlicer" << buffer.toHex(':');
        headerSSI *header = reinterpret_cast<headerSSI*>(buffer.data());
        int count = header->length - 3;
        if(port.bytesAvailable() >= count)
        {
            decodedData.append(port.read(count));
            decodedData.chop(2);  // отрежем чексум
        }
        if(!header->status.continuation)
        {
            emit barcodeReady(decodedData);
            decodedData.clear();
        }
    }
}
