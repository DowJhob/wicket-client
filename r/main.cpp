#include <QApplication>
#include <common_types.h>
//#include <controller.h>
#include <network.h>
#include <controller_2.h>
#include "test_timer.h"

//#define TEST

#include <barcode_reader/async_threaded_reader.h>

#include <picture2.h>

#ifdef NFC_ON
#include <NFC_copy.h>
#endif


int main(int argc, char *argv[])
{
    qRegisterMetaType<message>("message");
    QApplication a(argc, argv, false);      //https://forum.qt.io/topic/2002/linux-non-gui-application-drawimage-crash/6
    //Constructs an application object with argc command line arguments in argv.
    //If GUIenabled is true, a GUI application is constructed, otherwise a non-GUI
    //(console) application is created. I mean this third QApplication parameter.
    QThread::currentThread()->setPriority( QThread::TimeCriticalPriority );
    fprintf(stdout, "turnstile client - eulle@ya.ru\n");
    fflush(stdout);

    ///========================== OBJECTS =========================================
    controller _controller;
    network network_client;
    ///========================== lcd_display =========================================
    picture2 lcd_display(network_client.localIP);
//    libusb_async_reader barcode_reader;
//    QObject::connect( &barcode_reader, &libusb_async_reader::readyRead, &_controller,    &controller::send_barcode );
//    QObject::connect( &barcode_reader, &libusb_async_reader::log,       &network_client, &network::logger);
    lcd_display.start();
libusb_async_reader *barcode_reader;
    QThread thread;
    //    uchar      EP_IN = 0x81;
    uint16_t     VID = 0x05E0;
    uint16_t     PID = 0x1900;
    int iface = 0;
    int config = 1;
    int alt_config = 0;
    barcode_reader = new libusb_async_reader();
    barcode_reader->ini(VID, PID, iface, config, alt_config);
    QObject::connect(barcode_reader, &libusb_async_reader::readyRead_barcode,  &_controller, &controller::local_barcode);
    QObject::connect(barcode_reader, &libusb_async_reader::log,  &network_client, &network::logger);
    QObject::connect(&thread, &QThread::started, barcode_reader, &libusb_async_reader::init);
    barcode_reader->moveToThread(&thread);
    //barcode_reader->init();
    thread.start( );



    ///========================== controller =========================================
    QThread controller_thread;
    QObject::connect(&controller_thread, &QThread::started, &_controller, &controller::start);
    _controller.moveToThread(&controller_thread);
    QObject::connect( &_controller, SIGNAL(setPictureSIG(int, QString)), &lcd_display,    SLOT(setPicture(int, QString)));
    QObject::connect( &_controller, &controller::send_to_server,         &network_client, &network::SendToServer);
    QObject::connect( &_controller, &controller::log,                    &lcd_display,    &picture2::log);
    controller_thread.start(QThread::TimeCriticalPriority);
    ///========================== network =========================================
    QThread net_thread;
    QObject::connect(&net_thread,        &QThread::started, &network_client, &network::start);
    network_client.moveToThread(&net_thread);
    QObject::connect( &network_client, &network::log,                              &lcd_display, &picture2::log);
    QObject::connect( &network_client, &network::network_ready,                    &_controller, &controller::ext_provided_network_readySIG);
    QObject::connect( &network_client, &network::enter_STATE_server_search_signal, &_controller, &controller::ext_provided_server_searchSIG);
    QObject::connect( &network_client, &network::readyRead,                        &_controller, &controller::new_cmd_parse);
    net_thread.start(QThread::TimeCriticalPriority);

#ifdef TEST
test_timer tt;
QObject::connect(&tt, &test_timer::test, &network_client, &network::SendToServer);
#endif

    //===================== NFC =======================
#ifdef NFC_ON
    NFC nfc("/dev/ttyAPP0");
    QThread nfc_thread;
    QObject::connect(&nfc_thread, &QThread::started, &nfc,         &NFC::open);
    QObject::connect(&nfc,        &NFC::toPass,      &_controller, &controller::send_barcode );
    nfc.moveToThread(&nfc_thread);
    nfc_thread.start();
#endif

    QObject::connect(&_controller, &controller::exit, [&a](){qDebug() << "hop";//a.exit(42);
        a.closeAllWindows();}   );

    return a.exec();
}
