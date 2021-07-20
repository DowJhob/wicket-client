#ifndef ASYNC_H
#define ASYNC_H

#include "libusb.h"
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QSocketNotifier>
#include <QAbstractEventDispatcher>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

#include <poll.h>
#include "handler.h"
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
    handler h;
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
    bool running = false;
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
    int status = 0;
    barcode_msg data{};
    QMap<int, QByteArray> message{};
    static int LIBUSB_CALL hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
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
    static void LIBUSB_CALL intrrpt_cb_wrppr( libusb_transfer *transfer);

    bool init_libusb();
    bool device_init();
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

    void alloc_transfers(void);
    int set_param( unsigned char *_data, quint16 size, uint _timeout = 300)
    {
        quint16 send_value = 0x200 + _data[0];
        int rc = libusb_control_transfer( dev_handle, LIBUSB_RECIPIENT_INTERFACE
                                          | LIBUSB_REQUEST_TYPE_CLASS
                                          | LIBUSB_ENDPOINT_OUT, 9, send_value, 0, _data, size, _timeout );
        if (rc < 0)
            printf("set_param error: %s|%s\n", libusb_error_name(rc), libusb_strerror(rc));
        else if (rc < size)
            printf("set_param size error: %d%d\n", size, rc);
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

            }
        }
    }
    const libusb_pollfd **libusb_fd_list;
    pollfd *fds_list = nullptr;
    int count = 0;

public slots:
    void start()
    {
        dev_handle = h.dev_handle;
        connect(&h, &handler::SNAPI_msg_sig, this, &libusb_async_reader::parseSNAPImessage
        #ifdef BLOCK
                , Qt::BlockingQueuedConnection
        #else
                , Qt::QueuedConnection
        #endif
                );
        connect(&h, &handler::device_arrived_sig, this, &libusb_async_reader::SNAPI_scaner_init
        #ifdef BLOCK
                , Qt::BlockingQueuedConnection
        #else
                , Qt::QueuedConnection
        #endif
                );
        connect(&h, &handler::device_left_sig, this, &libusb_async_reader::disconnect_slot
        #ifdef BLOCK
                , Qt::BlockingQueuedConnection
        #else
                , Qt::QueuedConnection
        #endif
                );
        h.start();
        SNAPI_scaner_init();
        //            connect(this, &libusb_async_reader::handle_loop_sig, this, &libusb_async_reader::handle_loop_slot
        //                    , Qt::QueuedConnection
        //                    );
        //            connect(this, &libusb_async_reader::connect_sig, this, &libusb_async_reader::connect_slot
        //                    , Qt::QueuedConnection
        //                    );
        //            connect(this, &libusb_async_reader::disconnect_sig, this, &libusb_async_reader::disconnect_slot
        //                    , Qt::QueuedConnection
        //                    );
        //           qDebug() << "hop";
        //hop();
        //emit handle_loop_sig();
        //        qDebug() << "this thread " << thread();

        //  thread()->eventDispatcher()->processEvents(QEventLoop::AllEvents);
    }

    void parseSNAPImessage( barcode_msg data )
    {
        qDebug() << "reciever parseSNAPImessage: " << data.toHex(':');
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
    void connect_slot()
    {
        bool b = device_init();
        qDebug() << "connect_slot: " << b;
        if (b)
        {
            printf("Reader inited \n");
        }
        else
        {
            printf("Could not open USB device\n");
        }
    }
    void disconnect_slot()
    {
        if (dev_handle != nullptr)
        {
            printf("Disconnect reader \n");
            //libusb_free_pollfds(readerInstance->libusb_fd_list);
            libusb_release_interface(dev_handle, iface);
            libusb_close(dev_handle);
            dev_handle = nullptr;
            libusb_free_transfer(irq_transfer);
            //readerInstance->running = false;
            //delete(fds_list);
        }
    }
    void handle_loop_slot()
    {
        int rc;
        //   int p = poll(fds_list, count, 100);
        //        if (p >= 0 )
        {
            //rc = libusb_handle_events(nullptr);
            rc = libusb_handle_events_timeout(nullptr, &zero_tv);
            emit handle_loop_sig();
            // handle events from other sources here
        }
    }
    void hop()
    {
        int rc;
        /* Handle Events */
        qDebug() << "hop thread " << thread();
        while (true)
        {
            status = 0;
            //   int p = poll(fds_list, count, 100);
            //        if (p >= 0 )
            {
                //rc = libusb_handle_events(nullptr);
                rc = libusb_handle_events_timeout(nullptr, &zero_tv);
                switch (status) {
                case LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED : SNAPI_scaner_init(); break;
                case LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT : disconnect_slot(); break;
                    //case 3 : parseSNAPImessage(data); break;
                }
                thread()->eventDispatcher()->processEvents(QEventLoop::AllEvents);
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
    void SNAPI_scaner_init( );
signals:
    void handle_loop_sig();
    void SNAPI_msg_sig(barcode_msg);
    void connect_sig();
    void disconnect_sig();
    void readyRead_barcode(QByteArray);






    void init_completed();
    void log(QString);


};

#endif // ASYNC_H
