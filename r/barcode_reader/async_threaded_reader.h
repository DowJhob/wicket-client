#ifndef ASYNC_H
#define ASYNC_H

#include "libusb.h"
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QSocketNotifier>
#include <QAbstractEventDispatcher>
#include <poll.h>

#include "barcode_msg.h"

#define INTR_LENGTH		1024


class libusb_async_reader: public QObject
{
    Q_OBJECT
public:
    struct libusb_device_handle *dev_handle = nullptr;
    libusb_async_reader( )
    {
        qRegisterMetaType<barcode_msg>("barcode_msg");
        m_instance = this;
        //       connect(this, &libusb_async_reader::init_completed, this, &libusb_async_reader::SNAPI_scaner_init );
        //        connect(this, &libusb_async_reader::init_completed, this, &libusb_async_reader::start );
    }
    ~libusb_async_reader() Q_DECL_OVERRIDE
    {
        deleteLater();
        libusb_hotplug_deregister_callback(nullptr, callback_handle);
        libusb_exit(nullptr);
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

    //static
    struct libusb_transfer *irq_transfer;
    libusb_hotplug_callback_handle callback_handle;

    //struct libusb_transfer *bulk_transfer;
    int iface = 0;
    int config = 1;
    int alt_config;
    int completed=0;
    timeval zero_tv { 0, 10000 };
    bool barcode = false;
    unsigned char irqbuf[INTR_LENGTH];
    unsigned char bulkbuf[INTR_LENGTH];
    //    uchar SNAPI_SETUP[4]         { 0x80, 0x02, 0x00, 0x01 };
    uchar SNAPI_BARCODE_REQ[4]   { 0x01, 0x22, 0x01, 0x00 };
    uchar ENABLE_SCANNER[2]      { 0x06, 0x01 };
    uchar SNAPI_BEEP[32]         { 0x0d, 0x40, 0x00, 0x09, 0x00, 0x09, 0x05, 0x00, 0x17, 0x70, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    //0d    40    00    09    00    09    05    00    17    70    58   000000
    uchar SNAPI_BEEP2[10]        { 0x04, 0x1B, 0x21, 0x09, 0x04, 0x02, 0x00, 0x00, 0x02, 0x00 };
    uchar SNAPI_RET0[4]          { 0x01, 0x27, 0x01, 0x00 };
    uchar SNAPI_COMMAND_1[32]    { 0x0D, 0x40, 0x00, 0x06, 0x00, 0x06, 0x02, 0x00, 0x4E, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uchar SNAPI_INIT_1[32]       { 0x0D, 0x40, 0x00, 0x06, 0x00, 0x06, 0x20, 0x00, 0x04, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    barcode_msg data{};
    QMap<int, QByteArray> message{};
    static int LIBUSB_CALL hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data)
    {
        qDebug() << "hotplug_callback: ";
        //static libusb_device_handle *dev_handle = nullptr;
        //struct libusb_device_descriptor desc;

        int rc;
        int count = 0;
        //(void)libusb_get_device_descriptor(dev, &desc);  // not needed
        //libusb_open();
        libusb_async_reader *readerInstance = reinterpret_cast<libusb_async_reader*>(user_data);
        if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
            readerInstance->device_init();
            if (LIBUSB_SUCCESS != rc) {
                printf("Could not open USB device\n");
            }
        } else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
            if (readerInstance->dev_handle)
            {
                qDebug() << "LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT: ";
                libusb_close(readerInstance->dev_handle);
                readerInstance->dev_handle = nullptr;
                libusb_free_transfer(readerInstance->irq_transfer);
                libusb_free_pollfds(readerInstance->libusb_fd_list);
            }
        } else {
            printf("Unhandled event %d\n", event);
        }
        count++;

        return 0;
    }
    static void LIBUSB_CALL bulk_cb_wrppr( libusb_transfer *transfer)
    {
        //        if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
        //            qDebug() << "bulk_cb_wrppr transfer->status!" + QString::fromLatin1(libusb_error_name(transfer->status));
        //        QByteArray a = QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length );
        //        qDebug() << "bulk_cb_wrppr: " << a.toHex(':');

        //        libusb_async_reader *readerInstance = reinterpret_cast<libusb_async_reader*>(transfer->user_data);
        //        int rc = libusb_submit_transfer(readerInstance->bulk_transfer);
        //        if (rc != 0)
        //            qDebug() << "bulk_cb_wrppr submit_transfer err: " + QString::fromLatin1(libusb_error_name(rc));
        //        barcode_msg data;
        //        data.append( QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length ));
        //        //readerInstance->parseSNAPImessage( data );
        //        emit readerInstance->_tick( data );
    }
    static void LIBUSB_CALL intrrpt_cb_wrppr( libusb_transfer *transfer)
    {
        if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
            printf("intrrpt_cb_wrppr transfer->status! %s/n", libusb_error_name(transfer->status));
        QByteArray a = QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length );
        qDebug() << "intrrpt_cb_wrppr: " << a.toHex(':');
        libusb_async_reader *readerInstance = reinterpret_cast<libusb_async_reader*>(transfer->user_data);
        int rc = libusb_submit_transfer(readerInstance->irq_transfer);
        barcode_msg data;
        data.append( QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length ));
        //readerInstance->parseSNAPImessage( data );
        emit readerInstance->_tick( data );
        if (rc != 0)
            printf("intrrpt_cb_wrppr submit_transfer err: :%s\n", libusb_error_name(rc));
    }

    bool init_libusb()
    {
        int r = 0;
        //libusb_reset_device(devh);
        printf("%s - %d.%d.%d.%d\n", libusb_get_version()->describe, libusb_get_version()->major, libusb_get_version()->minor
               , libusb_get_version()->micro, libusb_get_version()->nano);
        if ( (r = libusb_init(nullptr)) != LIBUSB_SUCCESS )
            printf("libusb_init error: %s\n", libusb_error_name(r)) ;
        r = libusb_hotplug_register_callback(nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
                                             LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, VID, PID,
                                             LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, this,
                                             &callback_handle);
        if (LIBUSB_SUCCESS != r)
        {
            printf("Error creating a hotplug callback\n");
            libusb_exit(nullptr);
            return false;
        }
        return true;
    }
    bool device_init()
    {
        dev_handle = libusb_open_device_with_vid_pid( nullptr, VID, PID );
        if ( dev_handle != nullptr )
        {
            int r = libusb_detach_kernel_driver( dev_handle, iface );
            if(( r != LIBUSB_SUCCESS) && (r != LIBUSB_ERROR_NOT_FOUND ) )
                printf("libusb_detach_kernel_driver error: - %s\n", libusb_error_name(r));
            if((r = libusb_set_configuration(dev_handle, config)) != LIBUSB_SUCCESS)
                printf("NOT set configuration: - %s\n", libusb_error_name(r));
            if((r = libusb_claim_interface(dev_handle, iface)) != LIBUSB_SUCCESS)
                printf("libusb_claim_interface - %s\n", libusb_error_name(r));
            if((r = libusb_set_interface_alt_setting(dev_handle, iface, alt_config)) != LIBUSB_SUCCESS)
                printf("NOT set ALT configuration: - %s\n", libusb_error_name(r));
            alloc_transfers();
            SNAPI_scaner_init();
            fds();
            return true;
        }
        return false;
    }
    void fds()
    {
        libusb_fd_list = libusb_get_pollfds(nullptr);
        auto it = libusb_fd_list;

        for(;it != nullptr;++it)
            count++;
        it = libusb_fd_list;
        fds_list = new pollfd[count];
        for(;it != nullptr;++it)
        {
            fds_list[--count].fd = (*it)->fd;
            fds_list[--count].events = (*it)->events;
        }
    }
    void SNAPI_scaner_init( )
    {
        //--------Init usb--------------
        //qDebug() << "first libusb_control_transfer error: " <<
        int rc = libusb_control_transfer( dev_handle, (LIBUSB_RECIPIENT_INTERFACE | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT),
                                          10, 0, 0, nullptr, 0, 300 ) ;    //    rc = set_param( SNAPI_RET0, 4, 300 );
        //QThread::sleep(2);
        //--------Init usb 1------------
        set_param( SNAPI_INIT_1, 32, 300 );
        //QThread::sleep(2);
        //set_param( SNAPI_RET0, 4, 300 );
        set_param( ENABLE_SCANNER, 2, 300);
        //set_param(SNAPI_RET0, 4, 300);
        //QThread::sleep(2);
        //---------Init command 1------')
        set_param(SNAPI_COMMAND_1, 32, 300);
        //set_param(SNAPI_RET0, 4, 300);

        set_param( SNAPI_BEEP2, 10, 100 );
        //set_param( SNAPI_BEEP2, 10, 100 );
        //set_param( SNAPI_BEEP2, 10, 100 );
        //set_param( SNAPI_BEEP2, 10, 100 );
        //set_param( SNAPI_BEEP2, 10, 100 );
        //set_param( SNAPI_BEEP2, 10, 100 );
    }
    void alloc_transfers(void)
    {
        irq_transfer = libusb_alloc_transfer(0);
        //bulk_transfer = libusb_alloc_transfer(0);
        //if (!irq_transfer)
        //    return -ENOMEM;
        irq_transfer->user_data = this;
        //bulk_transfer->user_data = this;

        libusb_fill_interrupt_transfer(irq_transfer, dev_handle, EP_INTR, irqbuf, sizeof(irqbuf), intrrpt_cb_wrppr, this, 0);
        //libusb_fill_bulk_transfer(bulk_transfer, devh, EP_INTR, bulkbuf, sizeof(bulkbuf), bulk_cb_wrppr, this, 0);
        //        irq_transfer->flags = LIBUSB_TRANSFER_SHORT_NOT_OK
        //            | LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER;
        libusb_submit_transfer(irq_transfer);
        //libusb_submit_transfer(bulk_transfer);
    }
    int set_param( unsigned char *_data, quint16 size, uint _timeout = 300)
    {
        quint16 send_value = 0x200 + _data[0];
        int rc = libusb_control_transfer( dev_handle, LIBUSB_RECIPIENT_INTERFACE
                                          | LIBUSB_REQUEST_TYPE_CLASS
                                          | LIBUSB_ENDPOINT_OUT, 9, send_value, 0, _data, size, _timeout );
        if (rc < 0 || rc < size)
            printf("set_param error: %s|%s\n", libusb_error_name(rc), libusb_strerror(rc));
        //        emit log("set_param: " + QString::fromLatin1(libusb_error_name(rc)));
        return rc;
    }
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
                //set_param( SNAPI_BARCODE_REQ, 4, 100 );
                emit readyRead_barcode(aaa);
                //   emit _beep();
                beep();
                message.clear();
                qDebug() << "Barcode data decode: " << aaa;
            }
            else
            {
                //         int r;
                //         int transferred;
                ///         r = libusb_interrupt_transfer(devh, EP_INTR, irqbuf, INTR_LENGTH, &transferred, 1000);
                //        qDebug() << "Barcode data decode2222222222: " <<
                //                    QByteArray::fromRawData( reinterpret_cast<char*>
                //                (irqbuf), sizeof(irqbuf));
            }
        }
    }
    const libusb_pollfd **libusb_fd_list;
    pollfd *fds_list;
    int count = 0;

public slots:
    void start()
    {
        if ( init_libusb() )
        {
            //dev_handle = libusb_open_device_with_vid_pid( nullptr, VID, PID );
            if ( !device_init() )
            {
                return;
            }

            connect(this, &libusb_async_reader::_tick, this, &libusb_async_reader::parseSNAPImessage, Qt::QueuedConnection
                    );
            hop();
        }
        else
            qDebug() << "libusb_open_device_with_vid_pid error: device not open";
    }

    void parseSNAPImessage( barcode_msg data )
    {
        //qDebug() << "parseSNAPImessage: " << data.toHex(':');
        if ( !data.isNull() and data.size() >= 3 )
        {
            switch (data.at(0))
            {
            case 0x21 : //emit _ret();
                set_param( SNAPI_RET0, 4, 100 );
                break;
            case 0x22 :
                //emit _barcode_req(data);
                set_param( SNAPI_BARCODE_REQ, 4, 100 );
                parse_barcode( data ); break;
            case 0x27 : //emit _ret(data);
                set_param( SNAPI_RET0, 4, 100 );
                qDebug() << "ret";
                break;
            }
        }
    }
private slots:
    void hop()
    {
        int rc;
        /* Handle Events */
        while (true)
        {
            int p = poll(fds_list, count, 100);
            if (p >= 0 )
            {
                //rc = libusb_handle_events(nullptr);
                rc = libusb_handle_events_timeout(nullptr, &zero_tv);
                {
                    //     parseSNAPImessage( data );
                    thread()->eventDispatcher()->processEvents(QEventLoop::AllEvents);
                }
            }
            // handle events from other sources here
        }
    }
    void beep()
    {
        //        set_param( SNAPI_BARCODE_REQ, 4, 100 );

        set_param( SNAPI_BEEP2, 10, 100 );
        //        set_param( SNAPI_RET0, 4, 100 );
    }

signals:
    void readyRead_barcode(QByteArray);
    void init_completed();
    void log(QString);

    void _tick(barcode_msg);

};

#endif // ASYNC_H
