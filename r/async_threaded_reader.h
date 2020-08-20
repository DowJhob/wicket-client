#ifndef ASYNC_H
#define ASYNC_H

#include "libusb.h"
#include <QThread>
#include <QTimer>
#include <QDebug>

#define INTR_LENGTH		64

class async: public QObject {
    Q_OBJECT
public:

    QByteArray d;
    struct libusb_device_handle *devh = nullptr;
    async( uint16_t VID = 0x05E0, uint16_t PID = 0x1900, int iface = 0, int config = 1, int alt_config = 0, char EP_INTR = 0x81):
        VID( VID), PID( PID), EP_INTR(EP_INTR), iface ( iface), config (config), alt_config ( alt_config)
    {
        m_instance = this;
    }

protected:
    //async();
private:


    static async* m_instance;
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

    static void LIBUSB_CALL callback_wrapper(struct libusb_transfer *transfer)
    {
        async *connector = reinterpret_cast<async*>(transfer->user_data);
        connector->cb_irq(transfer);
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

public slots:
    void start()
    {
        int rc;
        /* Handle Events */
        while (true)
        {
            rc = libusb_handle_events_timeout(nullptr, &zero_tv);
            if (barcode)
            {
                set_param(  SNAPI_BARCODE_REQ, 4, 100 );
                barcode = false;
                //set_param( SNAPI_BEEP, 32, 100 );
                //set_param( SNAPI_RET0, 4, 100 );
            }
        }
    }
    void init()
    {
        int r = 0;
        //libusb_reset_device(devh);

        if ( (r = libusb_init(nullptr)) != LIBUSB_SUCCESS )
            qDebug() << "libusb_init error: " <<  libusb_error_name( r ) ;

        devh = libusb_open_device_with_vid_pid( nullptr, VID, PID );
        if ( devh != nullptr )
        {
            if((r = libusb_detach_kernel_driver( devh, iface ) ) != LIBUSB_SUCCESS )
                qDebug() << "libusb_detach_kernel_driver error:" << libusb_error_name( r );
            if((r = libusb_set_configuration(devh, config)) != 0)
                qDebug() << "NOT set configuration: " << libusb_error_name( r );
            if((r = libusb_claim_interface(devh, iface)) != 0)
                qDebug() << "libusb_claim_interface" << libusb_error_name( r );
            if((r = libusb_set_interface_alt_setting(devh, iface, alt_config)) != 0)
                qDebug() << "NOT set ALT configuration: " << libusb_error_name( r );
            alloc_transfers();
            emit init_completed();
        }
        else
            qDebug() << "libusb_open_device_with_vid_pid error: device not open";
    }
    void usb_init( )
    {
        //--------Init usb--------------
        //qDebug() << "first libusb_control_transfer error: " <<
        libusb_control_transfer( devh, LIBUSB_RECIPIENT_INTERFACE | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT,
                                 10, 0, 0, nullptr, 0, 300 ) ;
        //--------Init usb 1------------
        set_param( SNAPI_INIT_1, 32, 300 );
        set_param( SNAPI_RET0, 4, 300 );
        set_param( ENABLE_SCANNER, 2, 300);
        //---------Init command 1------')
        set_param(SNAPI_COMMAND_1, 32, 300);
        set_param(SNAPI_RET0, 4, 300);
        set_param(SNAPI_RET0, 4, 300);
    }
    void set_param( unsigned char *_data, quint16 size, uint _timeout=300)
    {
        quint16 send_value = 0x200 + _data[0];
        //qDebug() << "libusb_control_transfer error: " <<
        libusb_control_transfer( devh, LIBUSB_RECIPIENT_INTERFACE | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT, 9, send_value, 0, _data, size, _timeout );

    }
    void cb_irq(struct libusb_transfer *transfer)
    {
        QByteArray data;
        data = QByteArray::fromRawData( (char*)transfer->buffer, transfer->actual_length );

        m_instance->emitSignal( data );
    }
    void emitSignal(QByteArray data)
    {
        if ( !data.isNull() and data.size() == 32 and data[0] == (char)34 and data[1] == (char)1 )
        {
            qDebug() << "Barcode data decode2: " << data.mid(6, data.at(3));
            barcode = true;

            //m_instance->emitSignal( data.mid(6, data.at(3)) );
        }
        emit readyRead_barcode(data.mid(6, data.at(3)));
        libusb_submit_transfer(irq_transfer);
    }
signals:
    void readyRead_barcode(QByteArray);
    void init_completed( );
};
//==========================================================================================
class libusb_async_reader: public QObject {
    Q_OBJECT
public:
    struct libusb_device_handle *devh = nullptr;
    uchar      EP_IN = 0x81;
    uint16_t     VID = 0x05E0;
    uint16_t     PID = 0x1900;
    int iface = 0;
    int config = 1;
    int alt_config = 0;
    libusb_async_reader()
    {
        asyncc = new async( VID, PID, iface, config, alt_config);
        connect(asyncc, SIGNAL(readyRead_barcode(QByteArray)),  this, SLOT(readyRead_inner(QByteArray)), Qt::DirectConnection );
        connect(&thread, SIGNAL(started()), asyncc, SLOT(init()));
        connect(asyncc, SIGNAL(init_completed()), asyncc, SLOT(usb_init()) );
        connect(asyncc, SIGNAL(init_completed()), asyncc, SLOT(start()) );
        asyncc->moveToThread(&thread);
        thread.start( );
    }
    ~libusb_async_reader()
    {
        //delete interval_timer;
    }

signals:
    void readyRead(QByteArray);
private:
    QThread thread;
    async *asyncc;

public slots:
    void readyRead_inner(QByteArray data)
    {
        emit readyRead(data);
    }

};

#endif // ASYNC_H
