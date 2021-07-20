#include "async_threaded_reader.h"
libusb_async_reader* libusb_async_reader::m_instance = nullptr;

//libusb_async_reader* libusb_async_reader::dev_handle = nullptr;

int libusb_async_reader::hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    //qDebug() << "hot";
    libusb_async_reader *readerInstance = reinterpret_cast<libusb_async_reader*>(user_data);
    if(readerInstance == nullptr)
    {
        qDebug() << "readerInstance nullptr!!!";
        return 0;
    }

    if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event)
    {
        printf("Reader connect %d\n", readerInstance->status);
        if (readerInstance->dev_handle == nullptr)
        {
            readerInstance->status = event;
        readerInstance->device_init();
        }
    } else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
        //printf("Reader disconnect \n");
        readerInstance->status = event;
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
        }
    } else {
        printf("Unhandled event %d\n", event);
    }
    fflush(stdout);
    return 0;
}

void libusb_async_reader::intrrpt_cb_wrppr(libusb_transfer *transfer)
{
    if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
        printf("intrrpt_cb_wrppr transfer->status: %s\n", libusb_error_name(transfer->status));
    else{
        QByteArray a = QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length );
        qDebug() << "intrrpt_cb_wrppr: " << a.toHex(':');
        libusb_async_reader *readerInstance = reinterpret_cast<libusb_async_reader*>(transfer->user_data);
        int rc;
        if(readerInstance->irq_transfer)
            rc= libusb_submit_transfer(readerInstance->irq_transfer);
        barcode_msg data_;
        //readerInstance->data.clear();
        data_.append( QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length ));
        //readerInstance->data.append( QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length ));
        //readerInstance->parseSNAPImessage( data );
        //readerInstance->status = 3;
        emit readerInstance->SNAPI_msg_sig( data_ );
        if (rc != 0)
            printf("intrrpt_cb_wrppr submit_transfer err: :%s\n", libusb_error_name(rc));
    }
}

bool libusb_async_reader::init_libusb()
{
    int r = 0;
    printf("%s - %d.%d.%d.%d\n", libusb_get_version()->describe, libusb_get_version()->major, libusb_get_version()->minor
           , libusb_get_version()->micro, libusb_get_version()->nano);
    if ( (r = libusb_init(nullptr)) != LIBUSB_SUCCESS )
        printf("libusb_init error: %s\n", libusb_error_name(r)) ;

   r = libusb_hotplug_register_callback(nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
                                         LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                                         //LIBUSB_HOTPLUG_ENUMERATE,
                                         0,
                                         VID, PID,
                                         LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, this,
                                         &callback_handle);

    if (LIBUSB_SUCCESS != r)
    {
        printf("Error creating a hotplug callback\n");
        fflush(stdout);
        libusb_exit(nullptr);
        return false;
    }
    printf("hotplug callback created\n");
    fflush(stdout);

    return true;
}

bool libusb_async_reader::device_init()
{
    int r;
    printf("device_init\n");
    fflush(stdout);
    if (dev_handle != nullptr)
        return true;
    printf("device_init2\n");
    fflush(stdout);
    while ( (dev_handle = libusb_open_device_with_vid_pid( nullptr, VID, PID )) == nullptr )
    {
        printf("dont get device handle: \n");
        libusb_exit(nullptr);
        if ( (r = libusb_init(nullptr)) != LIBUSB_SUCCESS )
            printf("libusb_init error: %s\n", libusb_error_name(r)) ;
    }

    printf("device_getted handle\n");
    fflush(stdout);
    {


         r = LIBUSB_SUCCESS;
        if( libusb_kernel_driver_active(dev_handle, iface))
            r = libusb_detach_kernel_driver( dev_handle, iface );
        if(( r != LIBUSB_SUCCESS) )
            printf("libusb_detach_kernel_driver error: - %s\n", libusb_error_name(r));
        if((r = libusb_set_configuration(dev_handle, config)) != LIBUSB_SUCCESS)
            printf("NOT set configuration: - %s\n", libusb_error_name(r));
        if((r = libusb_set_interface_alt_setting(dev_handle, iface, alt_config)) != LIBUSB_SUCCESS )
            printf("NOT set ALT configuration: - %s\n", libusb_error_name(r));
        if((r = libusb_claim_interface(dev_handle, iface)) != LIBUSB_SUCCESS)
            printf("libusb_claim_interface - %s\n", libusb_error_name(r));

   //     qDebug() << "start alloc";
        alloc_transfers();
    //            qDebug() << "start snapi init";
     //   SNAPI_scaner_init();
       // qDebug() << "finish snapi init";
        //fds();
        return true;
    }
    printf("libusb_open_device_with_vid_pid: dont open device\n");
    return false;
}

void libusb_async_reader::SNAPI_scaner_init()
{
    //--------Init usb--------------
    qDebug() << "SNAPI_scaner_init: ";
    int rc = libusb_control_transfer( dev_handle, (LIBUSB_RECIPIENT_INTERFACE | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT),
                                      10, 0, 0, nullptr, 0, 300 ) ;    //    rc = set_param( SNAPI_RET0, 4, 300 );
    if (rc < 0)
        printf("first control_transfer error: %s|%s\n", libusb_error_name(rc), libusb_strerror(rc));

    //rc = libusb_handle_events_timeout(nullptr, &zero_tv);
    //QThread::sleep(2);
    //--------Init usb 1------------
    set_param( SNAPI_INIT_1, 32, 300 );
    //rc = libusb_handle_events_timeout(nullptr, &zero_tv);
    //QThread::sleep(2);
    //set_param( SNAPI_RET0, 4, 300 );
    set_param( ENABLE_SCANNER, 2, 300);
    //rc = libusb_handle_events_timeout(nullptr, &zero_tv);
    //set_param(SNAPI_RET0, 4, 300);
    //QThread::sleep(2);
    //---------Init command 1------')
    set_param(SNAPI_COMMAND_1, 32, 300);
    //rc = libusb_handle_events_timeout(nullptr, &zero_tv);
    //set_param(SNAPI_RET0, 4, 300);

    set_param( SNAPI_BEEP2, 10, 100 );
    //rc = libusb_handle_events_timeout(nullptr, &zero_tv);
    //set_param( SNAPI_BEEP2, 10, 100 );
    //set_param( SNAPI_BEEP2, 10, 100 );
    //set_param( SNAPI_BEEP2, 10, 100 );
    //set_param( SNAPI_BEEP2, 10, 100 );
    //set_param( SNAPI_BEEP2, 10, 100 );
}

void libusb_async_reader::alloc_transfers()
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
