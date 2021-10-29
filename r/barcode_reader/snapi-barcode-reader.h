#ifndef SNAPI_BARCODE_READER_H
#define SNAPI_BARCODE_READER_H

#include "libusb.h"
//#include <QThread>
//#include <QTimer>
#include <QDebug>
#include <QQueue>
//#include <QAbstractEventDispatcher>
//#include <QFutureWatcher>
//#include <QtConcurrent/QtConcurrent>

#include "libusb-wrapper.h"
#include "barcode_msg.h"

#define INTR_LENGTH		1024

struct comm{
    uchar* comm;
    int size;
};
class snapi_barcode_reader: public QObject
{
    Q_OBJECT
public:
    struct libusb_device_handle *dev_handle = nullptr;
    snapi_barcode_reader(uint16_t VID = 0x05E0, uint16_t PID = 0x1900, int iface = 0, int config = 1, int alt_config = 0, char EP_INTR = 0x81 )
    {
        qRegisterMetaType<barcode_msg>("barcode_msg");
        m_instance = this;
        this->VID = VID;
        this->PID = PID;
        this->iface = iface;
        this->config = config;
        this->alt_config = alt_config;
        this->EP_INTR = EP_INTR;
        //       connect(this, &libusb_async_reader::init_completed, this, &libusb_async_reader::SNAPI_scaner_init );
        //        connect(this, &libusb_async_reader::init_completed, this, &libusb_async_reader::start );
    }

protected:
    //async();
private:
    libusb_wrapper h;
    QTimer *loop;
    static snapi_barcode_reader* m_instance;
    uint16_t VID = 0x05E0;
    uint16_t PID = 0x1900;
    char EP_INTR;

    //static
    struct libusb_transfer *irq_transfer;
    libusb_hotplug_callback_handle callback_handle;

    //struct libusb_transfer *bulk_transfer;
    int iface = 0;
    int config = 1;
    int alt_config;
    int completed=0;
    timeval zero_tv { 0, 10000 };
    bool running = false;
    unsigned char irqbuf[INTR_LENGTH];
    unsigned char bulkbuf[INTR_LENGTH];

    //    uchar SNAPI_SETUP[4]         { 0x80, 0x02, 0x00, 0x01 };
    unsigned char SNAPI_INIT_1[32] {0x0D, 0x40, 0x00, 0x06, 0x00, 0x06, 0x20, 0x00, 0x04, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ENABLE_SCANNER[32] {0x06, 0x01};
    //=================================================
    unsigned char SNAPI_COMMAND_1[32] { 0x0D, 0x40, 0x00, 0x06, 0x00, 0x06, 0x02, 0x00, 0x4E, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char SNAPI_COMMAND_11[32] {0x0D, 0x40, 0x00, 0x10, 0x00, 0x10, 0x02, 0x00, 0x02, 0x15, 0x02, 0x16, 0x02, 0x17, 0x4E, 0x24, 0x4E, 0x27, 0x4E, 0x2B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char SNAPI_COMMAND_N [32] {0x0D, 0x40, 0x00, 0x04, 0x00, 0x04, 0x10, 0x00, 0x4E, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char SNAPI_COMMAND_NN [32] {0x0D, 0x40, 0x00, 0x04, 0x00, 0x04, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //=================================================

    unsigned char SNAPI_COMMAND_magic_1[32] { 0x0D, 0x40, 0x00, 0x09, 0x00, 0x09, 0x05, 0x00, 0x4E, 0x2A, 0x42, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char SNAPI_COMMAND_magic_2 [32] { 0x0D, 0x40, 0x00, 0x06, 0x00, 0x06, 0x02, 0x00, 0x13, 0x8D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char SNAPI_COMMAND_magic_3[32] {0x0D, 0x40, 0x00, 0x09, 0x00, 0x09, 0x05, 0x00, 0x4E, 0x2A, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    //=================================================
    //uchar SNAPI_BEEP[32]            { 0x0D, 0x40, 0x00, 0x09, 0x00, 0x09, 0x05, 0x00, 0x17, 0x70, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    unsigned char SNAPI_BEEP2[32] {0x04, 0x1B, 0x21, 0x09, 0x04, 0x02, 0x00, 0x00, 0x02, 0x00};

    unsigned char SNAPI_BARCODE_REQ[32] { 0x01, 0x22, 0x01, 0x00};
    unsigned char SNAPI_RET0[32] {0x01, 0x27, 0x01, 0x00};

    unsigned char SNAPI_IMAGE_JPG[32] {0x0B, 0x01, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char SNAPI_TRIGGER_MODE[32] {0x0B, 0x81, 0x02, 0x00, 0x8A, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uchar SNAPI_PULL_TRIGGER[2]     { 0x0A, 0x01};
    uchar SNAPI_RELEASE_TRIGGER[2]  { 0x0A, 0x00};

    QQueue<comm> command_queue{};

    int status = 0;
    barcode_msg data{};
    QMap<int, QByteArray> message{};

    int set_param( unsigned char *_data, quint16 size, uint _timeout = 300);
    void parse_barcode( barcode_msg data )
    {
        //
        if ( data.size() >= data.msg_size() // not nuul and larger n twoo bytes
             and data.total_msg_count() > 0   // message count not null
             and message.count() < data.total_msg_count() )         // stored count smaller n recieved
        {
            message.insert( data.msg_number(), data.mid(6, data.msg_size()));
            if( message.count() == data.total_msg_count() )    // Total messages
            {
                QByteArray aaa{};
                for(int i = 0; i < message.count(); i++)
                    aaa += message.value(i);
                emit readyRead_barcode(aaa);
                beep();
                message.clear();
                qDebug() << "Barcode data decode: " << aaa;
            }
            else
            {

            }
        }
    }
    void parse_sn( barcode_msg data )
    {
        qDebug() << "SN:" << data.mid(0x0F);
    }
public slots:
    void deque()
    {
        if(command_queue.isEmpty())
            return;
        comm c = command_queue.dequeue();
        set_param(c.comm, c.size, 500);

    }
    void start()
    {
        connect(&h, &libusb_wrapper::intrrpt_msg_sig, this, &snapi_barcode_reader::parseSNAPImessage, Qt::QueuedConnection);
        connect(&h, &libusb_wrapper::device_arrived_sig, this, &snapi_barcode_reader::SNAPI_scaner_init, Qt::QueuedConnection);
        connect(&h, &libusb_wrapper::device_left_sig, this, [this](){dev_handle = nullptr;}, Qt::QueuedConnection);
        h.start();
        SNAPI_scaner_init();
deque();
        //set_param(SNAPI_IMAGE_JPG, 32, 500);
        //set_param(SNAPI_TRIGGER_MODE, 32, 500);
        //set_param(SNAPI_PULL_TRIGGER, 2, 500);
        //set_param(SNAPI_RELEASE_TRIGGER, 2, 500);
    }
    void parseSNAPImessage( barcode_msg data )
    {
        //qDebug() << "reciever parseSNAPImessage: " << data;
        if ( !data.isNull() and data.size() >= 3 )
        {
            switch (data.at(0))
            {
            case 0x21 :
                deque();
                break;
            case 0x22 :
                set_param( SNAPI_BARCODE_REQ, 4, 100 );
                parse_barcode( data ); break;
            case 0x27 : if ( data.at(0x01) == 0x10)
                    parse_sn(data);
                set_param( SNAPI_RET0, 4, 100 );
                qDebug() << "ret";
                break;
            }
        }
    }

private slots:
    void beep()
    {
        set_param( SNAPI_BEEP2, 10, 100 );
    }
    void SNAPI_scaner_init( );

signals:
    void readyRead_barcode(QByteArray);

    void init_completed();
    void log(QString);
};

#endif // SNAPI_BARCODE_READER_H
