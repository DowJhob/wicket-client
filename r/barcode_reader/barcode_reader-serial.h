#ifndef BARCODE_READER-SERIAL_H
#define BARCODE_READER-SERIAL_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QSerialPortInfo>
#include <QSerialPort>

#include "qdevicewatcher/qdevicewatcher.h"

class barcode_reader_s : public QThread
{
    Q_OBJECT
public:
    barcode_reader_s();

protected:
    virtual bool event(QEvent *e);

private:
    QDeviceWatcher *watcher;

    void getAvalPrts();
    void open_prt();
    QString portName = "/dev/ttyACM0";
    uint16_t     VID = 0x05E0;
    uint16_t     PID = 0x1701;

    QSerialPort *port;

public slots:
    void slotDeviceAdded(const QString &dev);
    void slotDeviceRemoved(const QString &dev);
    void slotDeviceChanged(const QString &dev);

private slots:
    void packetSlicer();
    void errorHandler(QSerialPort::SerialPortError error);

signals:
    void barcode(QByteArray);
    void log(QString);
};

#endif // BARCODE_READER-SERIAL_H
