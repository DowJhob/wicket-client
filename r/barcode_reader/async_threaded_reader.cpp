#include "async_threaded_reader.h"
libusb_async_reader* libusb_async_reader::m_instance = nullptr;


void libusb_async_reader::SNAPI_scaner_init()
{
    dev_handle = h.dev_handle;
    //--------Init usb--------------
    qDebug() << "SNAPI_scaner_init: ";
    int rc = libusb_control_transfer( dev_handle, (LIBUSB_RECIPIENT_INTERFACE | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT),
                                      10, 0, 0, nullptr, 0, 300 ) ;    //    rc = set_param( SNAPI_RET0, 4, 300 );
    if (rc < 0)
        printf("first control_transfer error: %s|%s\n", libusb_error_name(rc), libusb_strerror(rc));

    //--------Init usb 1------------
    set_param( SNAPI_INIT_1, 32, 300 );
    set_param( ENABLE_SCANNER, 2, 300);
    //---------Init command 1------')
    set_param(SNAPI_COMMAND_1, 32, 300);
    set_param( SNAPI_BEEP2, 10, 100 );
}
