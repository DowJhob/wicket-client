#ifndef IBARCODE_H
#define IBARCODE_H

#include <QObject>

class IBarcode: public QObject
{
    Q_OBJECT
public:
    IBarcode();

signals:
    void barcodeReady(QByteArray);

};

#endif // IBARCODE_H
