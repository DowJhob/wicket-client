#include "libusb-wrapper.h"

libusb_wrapper* libusb_wrapper::m_instance = nullptr;

libusb_wrapper::libusb_wrapper()
{
    connect(this, &libusb_wrapper::loop_sig, this, &libusb_wrapper::loop, Qt::QueuedConnection);

    int r = 0;
    printf("%s - %d.%d.%d.%d", libusb_get_version()->describe, libusb_get_version()->major, libusb_get_version()->minor
           , libusb_get_version()->micro, libusb_get_version()->nano);
    if ( (r = libusb_init(nullptr)) != LIBUSB_SUCCESS )
    {
        //qDebug() << "libusb_init error: " << libusb_error_name(r);
        printf("libusb_init error: %s\n", libusb_error_name(r));
        return;
    }

    emit loop_sig();

    //device_init();
    //qDebug() << "libusb_wrapper " << thread();
    //cb_reg();
        //qDebug() << "libusb_init next";
    //
        //qDebug() << "libusb_init next2";
}

libusb_wrapper::~libusb_wrapper()
{
    //deleteLater();
    libusb_exit(nullptr);
//    QObject::~QObject();
}

void libusb_wrapper::run()
{
    //qDebug() << "hop thread " << thread();
    emit loop();
    //exec();
}

int libusb_wrapper::hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    printf("hotplug_callback \n");
    qDebug() << "hotplug_callback";
    libusb_wrapper *readerInstance = reinterpret_cast<libusb_wrapper*>(user_data);
    if(readerInstance == nullptr)
    {
        qDebug() << "readerInstance nullptr!!!";
        return 0;
    }
    if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event)
    {
        //qDebug() << "DEVICE_ARRIVED";
        printf("Reader connect \n");
        readerInstance->device_init();
        //emit readerInstance->device_arrived_sig();
    } else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
        if (readerInstance->dev_handle != nullptr)
        {
            printf("Disconnect reader \n");
            libusb_release_interface(readerInstance->dev_handle, readerInstance->iface);
            libusb_close(readerInstance->dev_handle);
            libusb_free_transfer(readerInstance->irq_transfer);
            libusb_free_pollfds(readerInstance->libusb_fd_list);
            //delete(readerInstance->fds_list);
            readerInstance->fds_list = nullptr;
            readerInstance->dev_handle = nullptr;
            emit readerInstance->device_left_sig();
        }
    } else {
        printf("Unhandled event %d\n", event);
    }
    fflush(stdout);
    return 0;
}

void libusb_wrapper::intrrpt_cb_wrppr(libusb_transfer *transfer)
{
    if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
        printf("intrrpt_cb_wrppr transfer->status: %s\n", libusb_error_name(transfer->status));
    else{
        QByteArray a = QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length );
        //qDebug() << "intrrpt_cb_wrppr: " << a.toHex(':');
        libusb_wrapper *readerInstance = reinterpret_cast<libusb_wrapper*>(transfer->user_data);
        int rc;
        if(readerInstance->irq_transfer)
            rc= libusb_submit_transfer(readerInstance->irq_transfer);
        barcode_msg data_;
        data_.append( a);
        emit readerInstance->intrrpt_msg_sig( data_ );
        if (rc != 0)
            printf("intrrpt_cb_wrppr submit_transfer err: :%s\n", libusb_error_name(rc));
    }
}

bool libusb_wrapper::device_init()
{
    int r = LIBUSB_SUCCESS;

    ///while ( (
    if((dev_handle = libusb_open_device_with_vid_pid( nullptr, VID, PID )) == nullptr )
    {
        printf("\ndont get device handle: libusb reinit\n");
        return false;
        //libusb_exit(nullptr);
        //usb_init();
    }

    printf(" -> device_getted handle= %p\n", dev_handle);

    if( libusb_kernel_driver_active(dev_handle, iface))
    {
        // when detach from linux and attach to libusb generate event LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED ??
        if(( r = libusb_detach_kernel_driver( dev_handle, iface )!= LIBUSB_SUCCESS))
        {
            printf("libusb_detach_kernel_driver error: - %s\n", libusb_error_name(r));
        return false;
        }
    }

    if((r = libusb_set_configuration(dev_handle, config)) != LIBUSB_SUCCESS)
    {
        printf("NOT set configuration: - %s\n", libusb_error_name(r));
        return false;
    }
    if((r = libusb_set_interface_alt_setting(dev_handle, iface, alt_config)) != LIBUSB_SUCCESS && r != LIBUSB_ERROR_NOT_FOUND)
    {
        printf("NOT set ALT configuration: - %s\n", libusb_error_name(r));
        return false;
    }
    if((r = libusb_claim_interface(dev_handle, iface)) != LIBUSB_SUCCESS)
    {
        printf("libusb_claim_interface - %s\n", libusb_error_name(r));
        return false;
    }


    irq_transfer = libusb_alloc_transfer(0);
    //bulk_transfer = libusb_alloc_transfer(0);
    //if (!irq_transfer)
    //    return -ENOMEM;
    irq_transfer->user_data = this;
    //bulk_transfer->user_data = this;
    libusb_fill_interrupt_transfer(irq_transfer, dev_handle, EP_INTR, irqbuf, sizeof(irqbuf), intrrpt_cb_wrppr, this, 0);
    //libusb_fill_bulk_transfer(bulk_transfer, dev_handle, 0x02, bulkbuf, sizeof(bulkbuf), bulk_cb_wrppr, this, 1000);
    //        irq_transfer->flags = LIBUSB_TRANSFER_SHORT_NOT_OK
    //            | LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER;
    if((r = libusb_submit_transfer(irq_transfer)) != LIBUSB_SUCCESS)
        printf("libusb_submit_transfer - %s\n", libusb_error_name(r));
    //libusb_submit_transfer(bulk_transfer);



    //int a =LIBUSB_ENDPOINT_IN;
    //a =LIBUSB_ENDPOINT_OUT;

    //fds();

    //emit loop_sig();


    emit device_arrived_sig();
    printf(" -> device_inited\n");
    fflush(stdout);
    return true;

    printf("libusb_open_device_with_vid_pid: dont open device\n");
    return false;
}

void libusb_wrapper::fds()
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
    qDebug() << "fds " << sizeof(libusb_fd_list) << count << libusb_fd_list;
}

void libusb_wrapper::loop()
{
    //qDebug() << "loop  ";
    int p = -1;
    //if(fds_list != nullptr)
    //    p = poll(fds_list, count, 100);
    //if (p >= 0 )
        //libusb_handle_events(nullptr);
        if ( int rc = libusb_handle_events_timeout(nullptr, &zero_tv) != LIBUSB_SUCCESS )
            printf("handle event error: %s|%s\n", libusb_error_name(rc), libusb_strerror(rc));
    //if(started)
        emit loop_sig();
}

void libusb_wrapper::cb_reg()
{
    int r = libusb_hotplug_register_callback(nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
                                             LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                                             //LIBUSB_HOTPLUG_ENUMERATE,
                                             0,
                                             VID, PID,
                                             LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, this,
                                             &callback_handle);

    if (LIBUSB_SUCCESS != r)
    {
        printf("\nError creating a hotplug callback\n");
        //libusb_exit(nullptr);
        //return false;
    }
    else
    {
        printf(" -> hotplug callback created\n");
        //fflush(stdout);
    }
}
