#ifndef HANDLER_H
#define HANDLER_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include "libusb.h"

#include "barcode_msg.h"

#define INTR_LENGTH		1024

class handler : public QThread
{
    Q_OBJECT
public:
    struct libusb_device_handle *dev_handle = nullptr;
    timeval zero_tv { 0, 10000 };
    handler();
    ~handler()Q_DECL_OVERRIDE
    {
        deleteLater();
        libusb_exit(nullptr);
    }
protected:
    void run() Q_DECL_OVERRIDE
    {
        int rc;
        /* Handle Events */
        qDebug() << "hop thread " << thread();
        //QTimer::singleShot(0, this, &handler::loop);
        emit loop();
        //       while (true)
        {
            //   int p = poll(fds_list, count, 100);
            //        if (p >= 0 )
            //rc = libusb_handle_events(nullptr);
            //            rc = libusb_handle_events_timeout(nullptr, &zero_tv);
            // handle events from other sources here
        }
        exec();
    }
private:
    struct libusb_transfer *irq_transfer;
    unsigned char irqbuf[INTR_LENGTH];
    libusb_hotplug_callback_handle callback_handle;
    static handler* m_instance;
    uint16_t VID = 0x05E0;
    uint16_t PID = 0x1900;
    char EP_INTR=0x81;
    int iface = 0;
    int config = 1;
    int alt_config=0;

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
    bool usb_init();
    void cb_reg();
    bool device_init();
private slots:
    void loop()
    {
        //rc =
        libusb_handle_events_timeout(nullptr, &zero_tv);
        //QTimer::singleShot(0, this, &handler::loop);
        emit loop();
        //qDebug() << "hop  ";
    }
signals:
    void device_arrived_sig();
    void device_left_sig();
    void SNAPI_msg_sig(barcode_msg);

    void loop_sig();
    void log(QString);

};

#endif // HANDLER_H
