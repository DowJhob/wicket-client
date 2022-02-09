#include "libusb-wrapper.h"

libusb_wrapper* libusb_wrapper::m_instance = nullptr;

libusb_wrapper::libusb_wrapper()
{
    connect(this, &libusb_wrapper::loop_sig, this, &libusb_wrapper::loop, Qt::QueuedConnection);
    if (!usb_init())
        return;
    device_init();
    fds();
}

libusb_wrapper::~libusb_wrapper()
{
    deleteLater();
    libusb_exit(nullptr);
}

void libusb_wrapper::run()
{
    //qDebug() << "hop thread " << thread();
    emit loop();
    exec();
}

bool libusb_wrapper::usb_init()
{
    int r = 0;
    printf("%s - %d.%d.%d.%d", libusb_get_version()->describe, libusb_get_version()->major, libusb_get_version()->minor
           , libusb_get_version()->micro, libusb_get_version()->nano);
    if ( (r = libusb_init(nullptr)) != LIBUSB_SUCCESS )
    {
        printf("libusb_init error: %s\n", libusb_error_name(r));
        return false;
    }
    cb_reg();
    return true;
}

int libusb_wrapper::hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    //qDebug() << "hot";
    libusb_wrapper *readerInstance = reinterpret_cast<libusb_wrapper*>(user_data);
    if(readerInstance == nullptr)
    {
        qDebug() << "readerInstance nullptr!!!";
        return 0;
    }
    if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event)
    {
        //printf("Reader connect %d\n", readerInstance->status);
        readerInstance->device_init();
        emit readerInstance->device_arrived_sig();
    } else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
        if (readerInstance->dev_handle != nullptr)
        {
            printf("Disconnect reader \n");
            //libusb_free_pollfds(readerInstance->libusb_fd_list);
            libusb_release_interface(readerInstance->dev_handle, readerInstance->iface);
            libusb_close(readerInstance->dev_handle);
            readerInstance->dev_handle = nullptr;
            libusb_free_transfer(readerInstance->irq_transfer);
            //readerInstance->running = false;
            //delete(fds_list);
            emit readerInstance->device_left_sig();
        }
    } else {
        printf("Unhandled event %d\n", event);
    }
    fflush(stdout);
    return 0;
}

//void libusb_wrapper::bulk_cb_wrppr(libusb_transfer *transfer)
//{
//            if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
//                qDebug() << "bulk_cb_wrppr transfer->status!" + QString::fromLatin1(libusb_error_name(transfer->status));
//            QByteArray a = QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length );
//            qDebug() << "bulk_cb_wrppr: " << a.toHex(':');

//            libusb_wrapper *readerInstance = reinterpret_cast<libusb_wrapper*>(transfer->user_data);
//            int rc = libusb_submit_transfer(readerInstance->bulk_transfer);
//            if (rc != 0)
//                qDebug() << "bulk_cb_wrppr submit_transfer err: " + QString::fromLatin1(libusb_error_name(rc));
//    //        barcode_msg data;
//    //        data.append( QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length ));
//    //        //readerInstance->parseSNAPImessage( data );
//    //        emit readerInstance->_tick( data );
//}

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
    int r;

    while ( (dev_handle = libusb_open_device_with_vid_pid( nullptr, VID, PID )) == nullptr )
    {
        printf("\ndont get device handle: libusb reinit\n");
        libusb_exit(nullptr);
        usb_init();
    }

    printf(" -> device_getted handle");

         r = LIBUSB_SUCCESS;
        if( libusb_kernel_driver_active(dev_handle, iface))
            r = libusb_detach_kernel_driver( dev_handle, iface );
        if(( r != LIBUSB_SUCCESS) )
            printf("libusb_detach_kernel_driver error: - %s\n", libusb_error_name(r));
        if((r = libusb_set_configuration(dev_handle, config)) != LIBUSB_SUCCESS)
            printf("NOT set configuration: - %s\n", libusb_error_name(r));
        if((r = libusb_set_interface_alt_setting(dev_handle, iface, alt_config)) != LIBUSB_SUCCESS && r != LIBUSB_ERROR_NOT_FOUND)
            printf("NOT set ALT configuration: - %s\n", libusb_error_name(r));
        if((r = libusb_claim_interface(dev_handle, iface)) != LIBUSB_SUCCESS)
            printf("libusb_claim_interface - %s\n", libusb_error_name(r));


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
        libusb_submit_transfer(irq_transfer);
        //libusb_submit_transfer(bulk_transfer);



        //int a =LIBUSB_ENDPOINT_IN;
        //a =LIBUSB_ENDPOINT_OUT;




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
}

void libusb_wrapper::loop()
{
    int p = poll(fds_list, count, 100);
    if (p >= 0 )
        //libusb_handle_events(nullptr);
        if ( int rc = libusb_handle_events_timeout(nullptr, &zero_tv) != LIBUSB_SUCCESS )
            printf("handle event error: %s|%s\n", libusb_error_name(rc), libusb_strerror(rc));
    emit loop();
    //qDebug() << "loop  ";
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
         libusb_exit(nullptr);
         //return false;
     }
 //    else
     {
     printf(" -> hotplug callback created");
     //fflush(stdout);
     }
}
