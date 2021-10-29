#ifndef LIBUSB_WRAPPER_H
#define LIBUSB_WRAPPER_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QTimer>

#include <poll.h>

#include "libusb.h"

#include "barcode_msg.h"

#define INTR_LENGTH		1024

class libusb_wrapper : public QThread
{
    Q_OBJECT
public:
    struct libusb_device_handle *dev_handle = nullptr;
    timeval zero_tv { 0, 10000 };
    libusb_wrapper();
    ~libusb_wrapper()Q_DECL_OVERRIDE
    {
        deleteLater();
        libusb_exit(nullptr);
    }
protected:
    void run() Q_DECL_OVERRIDE
    {
        qDebug() << "hop thread " << thread();
        emit loop();
        exec();
    }
private:
    struct libusb_transfer *irq_transfer;
    unsigned char irqbuf[INTR_LENGTH];
    //struct libusb_transfer *bulk_transfer;
    //unsigned char bulkbuf[1240000];

    libusb_hotplug_callback_handle callback_handle;
    static libusb_wrapper* m_instance;
    const libusb_pollfd **libusb_fd_list;
    pollfd *fds_list = nullptr;
    nfds_t count = 0;

    uint16_t VID = 0x05E0;
    uint16_t PID = 0x1900;
    char EP_INTR=0x81;
    int iface = 0;
    int config = 1;
    int alt_config=0;

    static int LIBUSB_CALL hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
    //static void LIBUSB_CALL bulk_cb_wrppr( libusb_transfer *transfer);
    static void LIBUSB_CALL intrrpt_cb_wrppr( libusb_transfer *transfer);
    bool usb_init();
    void cb_reg();
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

private slots:
    void loop()
    {
        int p = poll(fds_list, count, 100);
        if (p >= 0 )
        //libusb_handle_events(nullptr);
            if ( int rc = libusb_handle_events_timeout(nullptr, &zero_tv) != LIBUSB_SUCCESS )
                printf("handle event error: %s|%s\n", libusb_error_name(rc), libusb_strerror(rc));
        emit loop();
        //qDebug() << "loop  ";
    }
signals:
    void device_arrived_sig();
    void device_left_sig();
    void intrrpt_msg_sig(barcode_msg);

    void loop_sig();
    void log(QString);

};

#endif // LIBUSB_WRAPPER_H
