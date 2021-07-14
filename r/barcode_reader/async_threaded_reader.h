#ifndef ASYNC_H
#define ASYNC_H

#include "libusb.h"
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QSocketNotifier>

#define INTR_LENGTH		1024

class libusb_async_reader: public QObject
{
    Q_OBJECT
public:
    QByteArray d;
    struct libusb_device_handle *devh = nullptr;
    libusb_async_reader( )
    {
        m_instance = this;
        //       connect(this, &libusb_async_reader::init_completed, this, &libusb_async_reader::SNAPI_scaner_init );
        //        connect(this, &libusb_async_reader::init_completed, this, &libusb_async_reader::start );
    }
    void ini(uint16_t VID = 0x05E0, uint16_t PID = 0x1900, int iface = 0, int config = 1, int alt_config = 0, char EP_INTR = 0x81)
    {
        this->VID = VID;
        this->PID = PID;
        this->iface = iface;
        this->config = config;
        this->alt_config = alt_config;
        this->EP_INTR = EP_INTR;
    }
protected:
    //async();
private:
    QTimer *loop;
    static libusb_async_reader* m_instance;
    uint16_t VID = 0x05E0;
    uint16_t PID = 0x1900;
    char EP_INTR;
    struct libusb_transfer *irq_transfer = nullptr;
    int iface = 0;
    int config = 1;
    int alt_config;
    int completed=0;
    timeval zero_tv { 0, 200000 };
    bool barcode = false;
    unsigned char irqbuf[INTR_LENGTH];
    uchar SNAPI_SETUP[4]         { 0x80, 0x02, 0x00, 0x01 };
    uchar SNAPI_BARCODE_REQ[4]   { 0x01, 0x22, 0x01, 0x00 };
    uchar ENABLE_SCANNER[2]      { 0x06, 0x01 };
    uchar SNAPI_BEEP[32]         { 0x0d, 0x40, 0x00, 0x09, 0x00, 0x09, 0x05, 0x00, 0x17, 0x70, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uchar SNAPI_RET0[4]          { 0x01, 0x27, 0x01, 0x00 };
    uchar SNAPI_COMMAND_1[32]    { 0x0D, 0x40, 0x00, 0x06, 0x00, 0x06, 0x02, 0x00, 0x4E, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uchar SNAPI_INIT_1[32]       { 0x0D, 0x40, 0x00, 0x06, 0x00, 0x06, 0x20, 0x00, 0x04, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    QMap<int, QByteArray> message{};
    static void LIBUSB_CALL callback_wrapper(struct libusb_transfer *transfer)
    {
        libusb_async_reader *readerInstance = reinterpret_cast<libusb_async_reader*>(transfer->user_data);
        QByteArray data = QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length );
        readerInstance->parseSNAPImessage( data );
        int rc = libusb_submit_transfer(readerInstance->irq_transfer);
        if (rc != 0)
            readerInstance->log("callback_wrapper: " + QString::fromLatin1(libusb_error_name(rc)));
    }

    bool set_libusb()
    {
        int r = 0;
        //libusb_reset_device(devh);
        qDebug() << libusb_get_version()->describe << libusb_get_version()->major << libusb_get_version()->minor
                 << libusb_get_version()->micro << libusb_get_version()->nano;
        if ( (r = libusb_init(nullptr)) != LIBUSB_SUCCESS )
            qDebug() << "libusb_init error: " <<  libusb_error_name( r ) ;
        devh = libusb_open_device_with_vid_pid( nullptr, VID, PID );
        if ( devh != nullptr )
        {
            r = libusb_detach_kernel_driver( devh, iface );
            if(( r != LIBUSB_SUCCESS) && (r != LIBUSB_ERROR_NOT_FOUND ) )
                qDebug() << "libusb_detach_kernel_driver error:" << libusb_error_name( r );
            if((r = libusb_set_configuration(devh, config)) != LIBUSB_SUCCESS)
                qDebug() << "NOT set configuration: " << libusb_error_name( r );
            if((r = libusb_claim_interface(devh, iface)) != LIBUSB_SUCCESS)
                qDebug() << "libusb_claim_interface" << libusb_error_name( r );
            if((r = libusb_set_interface_alt_setting(devh, iface, alt_config)) != LIBUSB_SUCCESS)
                qDebug() << "NOT set ALT configuration: " << libusb_error_name( r );
           // libusb_pollfd
//            auto fds = libusb_get_pollfds(nullptr);
//            const struct libusb_pollfd **it = fds;
//            for(;*it != nullptr; ++it) {
//         qDebug() << "Adding fd: " << (*it)->fd << ", " << (*it)->events << endl;
//                auto fdddd = (*it)->fd;
//                if((*it)->events == 1)
//                {
//                QSocketNotifier *d = new QSocketNotifier(fdddd, QSocketNotifier::Read);
//                connect(d, &QSocketNotifier::activated, this, &libusb_async_reader::tac);
//            }}
            return true;
        }
        return false;
    }
    void SNAPI_scaner_init( )
    {
        //--------Init usb--------------
        //qDebug() << "first libusb_control_transfer error: " <<
        libusb_control_transfer( devh, LIBUSB_RECIPIENT_INTERFACE | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT,
                                 10, 0, 0, nullptr, 0, 300 ) ;
        //sleep(2)

        //--------Init usb 1------------
        set_param( SNAPI_INIT_1, 32, 300 );
        set_param( SNAPI_RET0, 4, 300 );
        set_param( ENABLE_SCANNER, 2, 300);
        //sleep(2)
        //---------Init command 1------')
        set_param(SNAPI_COMMAND_1, 32, 300);
        set_param(SNAPI_RET0, 4, 300);
        set_param(SNAPI_RET0, 4, 300);
    }
    void alloc_transfers(void)
    {
        irq_transfer = libusb_alloc_transfer(0);
        //if (!irq_transfer)
        //    return -ENOMEM;
        irq_transfer->user_data = this;
        libusb_fill_interrupt_transfer(irq_transfer, devh, EP_INTR, irqbuf, sizeof(irqbuf), callback_wrapper, this, 0);
        libusb_submit_transfer(irq_transfer);
    }
    void set_param( unsigned char *_data, quint16 size, uint _timeout=300)
    {
        quint16 send_value = 0x200 + _data[0];
        //qDebug() << "libusb_control_transfer error: " <<
        int rc = libusb_control_transfer( devh, LIBUSB_RECIPIENT_INTERFACE
                                          | LIBUSB_REQUEST_TYPE_CLASS
                                          | LIBUSB_ENDPOINT_OUT, 9, send_value, 0, _data, size, _timeout );
        if (rc < 0)
            emit log("set_param: " + QString::fromLatin1(libusb_error_name(rc)));
    }
    void parseSNAPImessage( QByteArray data )
    {
        if ( !data.isNull() and data.size() >= 32 and static_cast<int>(data[0]) == 34 //
             and static_cast<int>(data[1]) > 0 and message.count() < data[1] )
        {
            message.insert( data[2], data.mid(6, data.at(3)));
            if( message.count() == data[1] )    // Total messages
            {
                QByteArray aaa{};
                for(int i = 0; i < message.count(); i++)
                    aaa += message.value(i);
                readyRead_barcode(aaa);
                message.clear();
                qDebug() << "Barcode data decode2: " << aaa;
                set_param( SNAPI_BEEP, 32, 100 );
                set_param( SNAPI_RET0, 4, 100 );
            }
        }
    }

public slots:
    void init()
    {
        connect(this, &libusb_async_reader::_loop, this, &libusb_async_reader::tick);
        if ( set_libusb() )
        {
            alloc_transfers();
            SNAPI_scaner_init();
            emit tick();
            //emit init_completed();
        }
        else
            qDebug() << "libusb_open_device_with_vid_pid error: device not open";
    }
private slots:
    void tick()
    {
        //qDebug() << "start";
        int rc;
        set_param( SNAPI_BARCODE_REQ, 4, 100 );
        //    rc = libusb_handle_events_timeout(nullptr, &zero_tv);
        rc = libusb_handle_events(nullptr);
        if (rc != 0)
            emit log("start: " + QString::fromLatin1(libusb_error_name(rc)));
        emit _loop();
    }
signals:
    void readyRead_barcode(QByteArray);
    void init_completed();
    void log(QString);

    void _loop();
};

#endif // ASYNC_H
