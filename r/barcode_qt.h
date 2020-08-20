#ifndef BARCODE_QT_H
#define BARCODE_QT_H
#include <fcntl.h>
#include <unistd.h>
#include <async_threaded_reader.h>
#include <QSocketNotifier>
#include <QTimer>
#include <QFile>
#include <QDebug>

#include <common_types.h>

class barcode_qt: public QObject {
    Q_OBJECT
public:
    QTimer test;

    QString barcode;
//    QString exit_barcode;
    bool CDC = true;                   // type barcode reader true CDC mode, false hidraw

    void set_test()
    {
        QObject::connect(&test, SIGNAL(timeout()), this, SLOT(test_slot()));
        test.start(3000);
    }
    void set_normal()
    {
        QObject::disconnect(&test, SIGNAL(timeout()), this, SLOT(test_slot()));
        test.stop();
    }
    barcode_qt( bool CDC = true )
    {

        this->CDC = CDC;                   // type barcode reader true CDC mode, false hidraw
        if ( CDC )
            barcode_scaner_device = "/dev/ttyACM0";
        else
            barcode_scaner_device = "/dev/hidraw0";


        //("/dev/usb/hiddev0");
        // =  ("/dev/char/247\:0");
        // =  ("/sys/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.3/1-1.3:1.0/0003:05E0:1900.0001");
        // =  ("/dev/serial/by-path/platform-3f980000.usb-usb-0:1.3:1.0");
        // =  ("/dev/serial/by-path/platform-3f980000.usb-usb-0:1.4:1.0");
        // =  ("/dev/serial/by-path/platform-3f980000.usb-usb-0:1.3:1.0");
        // =  ("/sys/dev/char/189\:140");
//connect();

    }
    void connect()
    {
        //entry_fd = open(barcode_scaner_device, O_RDWR|O_NONBLOCK);
        if( entry_fd == -1 )
            fprintf(stdout, "can not open barcode device %s\n", barcode_scaner_device.toStdString().c_str());
        else
        {
            fprintf(stdout, "open barcode device %s\n", barcode_scaner_device.toStdString().c_str());
            entry_notifer = new QSocketNotifier( entry_fd, QSocketNotifier::Read);
            QObject::connect( entry_notifer, SIGNAL(activated(int)), this, SLOT(entry_barcode_recieve()));
        }
        fflush(stdout);
    }
    void snapi_connect()
    {
        usb = new libusb_async_reader();
        QObject::connect( usb, SIGNAL(readyRead(QByteArray)), this, SLOT(read_barcode_snapi(QByteArray)) );
    }
    void setDeviceBarcodeName(QByteArray barcode_scaner_device)
    {
        this->barcode_scaner_device  = barcode_scaner_device;
    }
    void _close()
    {
        close(entry_fd);
    }
signals:
    void barcode_recive();
private slots:
void test_slot()
{
    //qDebug() << "test_slot:" << " / remainingTime: " + QString::number( test.remainingTime() );
    barcode = "9780201379624";
    emit barcode_recive();
}
    void read_barcode()
    {
        if ( CDC )                   // type barcode reader true CDC mode, false hidraw
            read_barcode_CDC( );
        else
            read_barcode_hidraw();
//      read_SNAPI_barcode(direction_state::entry);
    }
    void read_barcode_CDC()
    {
        qint64 nbytes = read(entry_fd, &barcode_buff, sizeof(barcode_buff));
        if(nbytes > 0)
            for (int i = 0; i < nbytes; i++)
            {
                if (barcode_buff[i] == '\n')
                {
//                    if (direction == direction_state::dir_entry)
                        barcode = append_string;
//                    if (direction == direction_state::dir_exit)
//                        exit_barcode = append_string;
                    append_string.clear();
                    emit barcode_recive();
                    return;
                }
                append_string.append(barcode_buff[i]);
            }
    }
    void read_barcode_hidraw()
    {
        // https://wiki.osdev.org/USB_Human_Interface_Devices
        char letter_adder = 0;
        char digit_adder = 0;
        qint64 nbytes = read(entry_fd, &barcode_buff, 8);
        qDebug() << "barcode_buff[2] = " << QString::number( barcode_buff[2],16) ;
        if ( nbytes > 0 )
        {
            if ( barcode_buff[2] == 0 )
            {
//                if (direction == direction_state::dir_entry)
                    barcode = append_string;
//                if (direction == direction_state::dir_exit)
//                    exit_barcode = append_string;
                emit barcode_recive();
                qDebug() << "append_string = " << append_string;
                append_string.clear();
                return;
            }
            if ( barcode_buff[0] == 2 )
            {
                letter_adder = 0x3D;
                digit_adder = 0;
            }
            else{
                letter_adder = 0x5D;
                digit_adder = 0x13;
            }
            if ( (barcode_buff[2] >= 0x04) && (barcode_buff[2] <= 0x1D) )    //letters
                append_string += (char)( barcode_buff[2] + letter_adder );
            if ( (barcode_buff[2] >= 0x1E) && (barcode_buff[2] <= 0x26) )    //digits
                append_string += (char)( barcode_buff[2] + digit_adder );
            if ( barcode_buff[2] == 0x27 )                                 //zero
                append_string += (char)0x30;
        }
    }
    void read_barcode_snapi(QByteArray _data)
    {
        barcode = _data;
        emit barcode_recive();
    }
private:
    libusb_async_reader *usb;
    QByteArray barcode_scaner_device;
    QSocketNotifier *entry_notifer;
    QFile barcode_scaner_device_desc;
    int entry_fd, ctrl_fd;
    char barcode_buff[8];
    QString append_string{};
};

#endif // BARCODE_H
