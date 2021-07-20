#include "handler.h"

handler* handler::m_instance = nullptr;

handler::handler()
{
 //   int r = 0;
    if (!usb_init())
        return;
    device_init();
   // }
}

bool handler::usb_init()
{
    int r = 0;
    printf("%s - %d.%d.%d.%d", libusb_get_version()->describe, libusb_get_version()->major, libusb_get_version()->minor
           , libusb_get_version()->micro, libusb_get_version()->nano);
    if ( (r = libusb_init(nullptr)) != LIBUSB_SUCCESS )
    {
        printf("libusb_init error: %s\n", libusb_error_name(r));
        return false;
    }
    return true;
}

int handler::hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    //qDebug() << "hot";
    handler *readerInstance = reinterpret_cast<handler*>(user_data);
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
        }
        emit readerInstance->device_left_sig();
    } else {
        printf("Unhandled event %d\n", event);
    }
    fflush(stdout);
    return 0;
}

void handler::intrrpt_cb_wrppr(libusb_transfer *transfer)
{
    if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
        printf("intrrpt_cb_wrppr transfer->status: %s\n", libusb_error_name(transfer->status));
    else{
        QByteArray a = QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length );
        qDebug() << "intrrpt_cb_wrppr: " << a.toHex(':');
        handler *readerInstance = reinterpret_cast<handler*>(transfer->user_data);
        int rc;
        if(readerInstance->irq_transfer)
            rc= libusb_submit_transfer(readerInstance->irq_transfer);
        barcode_msg data_;
        //readerInstance->data.clear();
        data_.append( a);
        //readerInstance->data.append( QByteArray::fromRawData( reinterpret_cast<char*>(transfer->buffer), transfer->actual_length ));
        //readerInstance->parseSNAPImessage( data );
        //readerInstance->status = 3;
        emit readerInstance->SNAPI_msg_sig( data_ );
        if (rc != 0)
            printf("intrrpt_cb_wrppr submit_transfer err: :%s\n", libusb_error_name(rc));
    }
}

bool handler::device_init()
{
    int r;

    while ( (dev_handle = libusb_open_device_with_vid_pid( nullptr, VID, PID )) == nullptr )
    {
        printf("\ndont get device handle: libusb reinit\n");
        libusb_exit(nullptr);
        usb_init();
    }

    printf(" -> device_getted handle");

   r = libusb_hotplug_register_callback(nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
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
        //libusb_fill_bulk_transfer(bulk_transfer, devh, EP_INTR, bulkbuf, sizeof(bulkbuf), bulk_cb_wrppr, this, 0);
        //        irq_transfer->flags = LIBUSB_TRANSFER_SHORT_NOT_OK
        //            | LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER;
        libusb_submit_transfer(irq_transfer);
        //libusb_submit_transfer(bulk_transfer);
        printf(" -> device_inited\n");
        fflush(stdout);
        return true;

    printf("libusb_open_device_with_vid_pid: dont open device\n");
    return false;
}
