#include "barcode_reader-serial.h"

barcode_reader::barcode_reader()
{
    enumr = new QextSerialEnumerator;
    connect(enumr, &QextSerialEnumerator::deviceDiscovered, this, &barcode_reader::arrived);
    connect(enumr, &QextSerialEnumerator::deviceRemoved, this, &barcode_reader::removed);
    enumr->setUpNotifications();
}

void barcode_reader::arrived(const QextPortInfo &info)
{
    if( info.productID == productID && info.vendorID == vendorID)
        portName = info.portName;
}

void barcode_reader::removed(const QextPortInfo &info)
{
    if( info.productID == productID && info.vendorID == vendorID)
        portName.clear();
}
