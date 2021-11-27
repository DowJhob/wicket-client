#include "snapi-barcode-reader.h"

snapi_barcode_reader* snapi_barcode_reader::m_instance = nullptr;

snapi_barcode_reader::snapi_barcode_reader(uint16_t VID, uint16_t PID, int iface, int config, int alt_config, char EP_INTR)
{
    qRegisterMetaType<barcode_msg>("barcode_msg");
    m_instance = this;
    this->VID = VID;
    this->PID = PID;
    this->iface = iface;
    this->config = config;
    this->alt_config = alt_config;
    this->EP_INTR = EP_INTR;
    //       connect(this, &libusb_async_reader::init_completed, this, &libusb_async_reader::SNAPI_scaner_init );
    //        connect(this, &libusb_async_reader::init_completed, this, &libusb_async_reader::start );
}

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

void snapi_barcode_reader::parse_barcode(barcode_msg data)
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
            emit readyRead_barcode(aaa);
            beep();
            message.clear();
            qDebug() << "Barcode data decode: " << aaa;
        }
        else
        {

        }
    }
}

void snapi_barcode_reader::parse_sn(barcode_msg data)
{
    qDebug() << "SN:" << data.mid(0x0F);
}

void snapi_barcode_reader::deque()
{
    if(command_queue.isEmpty())
        return;
    comm c = command_queue.dequeue();
    set_param(c.comm, c.size, 500);

}

void snapi_barcode_reader::start()
{
    connect(&h, &libusb_wrapper::intrrpt_msg_sig, this, &snapi_barcode_reader::parseSNAPImessage, Qt::QueuedConnection);
    connect(&h, &libusb_wrapper::device_arrived_sig, this, &snapi_barcode_reader::SNAPI_scaner_init, Qt::QueuedConnection);
    connect(&h, &libusb_wrapper::device_left_sig, this, [this](){dev_handle = nullptr;}, Qt::QueuedConnection);
    h.start();
    SNAPI_scaner_init();
    deque();
    //set_param(SNAPI_IMAGE_JPG, 32, 500);
    //set_param(SNAPI_TRIGGER_MODE, 32, 500);
    //set_param(SNAPI_PULL_TRIGGER, 2, 500);
    //set_param(SNAPI_RELEASE_TRIGGER, 2, 500);
}

void snapi_barcode_reader::parseSNAPImessage(barcode_msg data)
{
    //qDebug() << "reciever parseSNAPImessage: " << data;
    if ( !data.isNull() and data.size() >= 3 )
    {
        switch (data.at(0))
        {
        case 0x21 :
            deque();
            break;
        case 0x22 :
            set_param( SNAPI_BARCODE_REQ, 4, 100 );
            parse_barcode( data ); break;
        case 0x27 : if ( data.at(0x01) == 0x10)
                parse_sn(data);
            set_param( SNAPI_RET0, 4, 100 );
            qDebug() << "ret";
            break;
        }
    }
}

void snapi_barcode_reader::beep()
{
    set_param( SNAPI_BEEP2, 10, 100 );
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
