#ifndef BARCODE_READER-SERIAL_H
#define BARCODE_READER-SERIAL_H

#include <QObject>
#include  "qextserial/qextserialenumerator.h"

class barcode_reader : public QObject
{
    Q_OBJECT
public:
    QString productID = "0000";
    QString vendorID = "0000";

    barcode_reader();
    QString portName{};

    QextSerialEnumerator *enumr;

public slots:
    void arrived(const QextPortInfo &info);
    void removed(const QextPortInfo &info);

signals:
    void readyRead_barcode(QByteArray);

    void init_completed();
    void log(QString);
};

#endif // BARCODE_READER-SERIAL_H
