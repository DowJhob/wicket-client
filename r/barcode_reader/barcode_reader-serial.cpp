#include "barcode_reader-serial.h"

barcode_reader_s::barcode_reader_s()
{
    connect(this, &QThread::started, this, [this](){
        port = new QSerialPort;
        port->moveToThread(this);
        connect(port, &QSerialPort::readyRead, this, &barcode_reader_s::packetSlicer);
        connect(port, &QSerialPort::errorOccurred, this, &barcode_reader_s::errorHandler);
        getAvalPrts();
        open_prt();
    });
    start();

    moveToThread(this); //Let bool event(QEvent *e) be in another thread
    watcher = new QDeviceWatcher;
    watcher->moveToThread(this);
    watcher->appendEventReceiver(this);
    //    connect(watcher, &QDeviceWatcher::deviceAdded, this, &barcode_reader_s::slotDeviceAdded, Qt::DirectConnection);
    //    connect(watcher, &QDeviceWatcher::deviceChanged, this, &barcode_reader_s::slotDeviceChanged, Qt::DirectConnection);
    //    connect(watcher, &QDeviceWatcher::deviceRemoved, this, &barcode_reader_s::slotDeviceRemoved, Qt::DirectConnection);
    watcher->start();






    //
}

bool barcode_reader_s::event(QEvent *e)
{
    if (e->type() == QDeviceChangeEvent::registeredType()) {
        QDeviceChangeEvent *event = (QDeviceChangeEvent *) e;
        QString action("Change");
        if (event->action() == QDeviceChangeEvent::Add)
        {
            if(event->device().contains("dev/ttyACM"))
            {
                portName = event->device();
                open_prt();
            }
            action = "Add";
        }
        else if (event->action() == QDeviceChangeEvent::Remove)
        {
            if(event->device().contains("dev/ttyACM"))
            {
                portName = event->device();
                port->close();
            }
            action = "Remove";
        }

        qDebug("tid=%#x event=%d %s: %s %s",
               (quintptr) QThread::currentThreadId(),
               e->type(),
               __PRETTY_FUNCTION__,
               qPrintable(action),
               qPrintable(event->device()));
        event->accept();
        return true;
    }
    return QObject::event(e);
}

void barcode_reader_s::getAvalPrts()
{
    auto ports = QSerialPortInfo::availablePorts();
    for( auto it : ports )
    {
//        qDebug() << "ALL port found:" << it.description() << it.vendorIdentifier() << it.productIdentifier()
//                 << it.manufacturer() << it.portName() << it.systemLocation();
        if( it.portName().contains("ttyACM") )
        {
            portName = it.portName();
            qDebug() << "port found:" << portName;
            break;
        }
    }
}

void barcode_reader_s::open_prt()
{
    //    getAvalPrts();
    port->setPortName(portName);

//    qDebug() << "open_prt";
    bool op = port->open(QIODevice::ReadWrite);
    port->clear(QSerialPort::Direction::AllDirections);
    qDebug() << (op?"port open":"portNOT open!");
}

void barcode_reader_s::slotDeviceAdded(const QString &dev)
{
    qDebug("tid=%#x %s: add %s",
           (quintptr) QThread::currentThreadId(),
           __PRETTY_FUNCTION__,
           qPrintable(dev));
}

void barcode_reader_s::slotDeviceRemoved(const QString &dev)
{
    qDebug("tid=%#x %s: remove %s",
           (quintptr) QThread::currentThreadId(),
           __PRETTY_FUNCTION__,
           qPrintable(dev));
}

void barcode_reader_s::slotDeviceChanged(const QString &dev)
{
    qDebug("tid=%#x %s: change %s",
           (quintptr) QThread::currentThreadId(),
           __PRETTY_FUNCTION__,
           qPrintable(dev));
}

void barcode_reader_s::packetSlicer()
{
    emit barcode(port->readAll());
}

void barcode_reader_s::errorHandler(QSerialPort::SerialPortError error)
{
    qDebug() << "error" << error;
}
