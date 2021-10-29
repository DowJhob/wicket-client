#include "snapi-barcode-reader.h"
snapi_barcode_reader* snapi_barcode_reader::m_instance = nullptr;


int snapi_barcode_reader::set_param(uchar *_data, quint16 size, uint _timeout)
{
    quint16 send_value = 0x200 + _data[0];

    uint8_t request_type = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;
    //uint8_t request_type = LIBUSB_REQUEST_TYPE_CLASS ;
    uint8_t request = LIBUSB_REQUEST_SET_CONFIGURATION ;
    //LIBUSB_REQUEST_SET_INTERFACE;
    int rc = libusb_control_transfer( dev_handle, request_type, request, send_value, 0, _data, size, _timeout );
    if (rc < 0)
        printf("set_param error: %s|%s\n", libusb_error_name(rc), libusb_strerror(rc));
    else if (rc < size)
        printf("set_param size error: %d%d\n", size, rc);
    //        emit log("set_param: " + QString::fromLatin1(libusb_error_name(rc)));
    //QThread::sleep(2);

    return rc;
}

void snapi_barcode_reader::SNAPI_scaner_init()
{
    dev_handle = h.dev_handle;
    //--------Init usb--------------
    qDebug() << "SNAPI_scaner_init: ";
    int rc = 0;

    //uchar data[4096]{};


    int ss = LIBUSB_REQUEST_GET_STATUS ;

    uint8_t request_type = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;
    //uint8_t request_type = LIBUSB_REQUEST_TYPE_CLASS ;
    uint8_t request = LIBUSB_REQUEST_GET_INTERFACE ;
    rc = libusb_control_transfer( dev_handle, request_type, request, 0, 0, nullptr, 0, 300 ) ;
    if (rc < 0)
        printf("first control_transfer error: %s|%s\n", libusb_error_name(rc), libusb_strerror(rc));

    //for (uint8_t desc_index =1; desc_index<10; desc_index++)
    //{
    //if ( (rc = libusb_get_string_descriptor_ascii(dev_handle, desc_index, data, sizeof(data) ) < 0))
    //printf("libusb_get_descriptor error: %s|%s\n", libusb_error_name(rc), libusb_strerror(rc));
    //else
    //qDebug() << "data: " << QByteArray::fromRawData((char*)data, rc);
    //}

//    //--------Init usb 1------------
//    set_param( SNAPI_INIT_1, 32, 300 );
//    set_param( ENABLE_SCANNER, 2, 300);
//    //---------Init command 1------')
//    //set_param(SNAPI_COMMAND_1, 32, 300);
//    set_param(SNAPI_COMMAND_NN, 32, 300);
//    set_param(SNAPI_COMMAND_magic_1, 32, 300);
//    set_param(SNAPI_COMMAND_magic_2, 32, 300);
//    set_param(SNAPI_COMMAND_magic_3, 32, 300);
//    set_param( SNAPI_BEEP2, 10, 100 );

    //--------Init usb 1------------
    comm c;
    c.comm = SNAPI_INIT_1;
    c.size = 32;
    command_queue.enqueue(c);

    c.comm = ENABLE_SCANNER;
    c.size = 2;
    command_queue.enqueue(c);
    //---------Init command 1------')
    //set_param(SNAPI_COMMAND_1, 32, 300);

    c.comm = SNAPI_COMMAND_NN;
    c.size = 32;
    command_queue.enqueue(c);

    c.comm = SNAPI_COMMAND_magic_1;
    c.size = 32;
    command_queue.enqueue(c);

    c.comm = SNAPI_COMMAND_magic_2;
    c.size = 32;
    command_queue.enqueue(c);

    c.comm = SNAPI_COMMAND_magic_3;
    c.size = 32;
    command_queue.enqueue(c);

    c.comm = SNAPI_BEEP2;
    c.size = 32;
    command_queue.enqueue(c);
}
