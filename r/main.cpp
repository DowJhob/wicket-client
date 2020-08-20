#include <QApplication>
#include <common_types.h>
//#include <controller.h>
#include <controller_2.h>
#include <async_threaded_reader.h>
#include <picture2.h>
#include <network.h>
#include <NFC.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv, false);      //https://forum.qt.io/topic/2002/linux-non-gui-application-drawimage-crash/6
                                            //Constructs an application object with argc command line arguments in argv.
                                            //If GUIenabled is true, a GUI application is constructed, otherwise a non-GUI
                                            //(console) application is created. I mean this third QApplication parameter.
    QThread::currentThread()->setPriority( QThread::TimeCriticalPriority );
    fprintf(stdout, "turnstile client - eulle@ya.ru\n");
    fflush(stdout);

///========================== THREADS =========================================
    QThread controller_thread;
    QThread net_thread;
    QThread picture_thread;

///========================== OBJECTS =========================================
    controller _controller;
    network network_client;
    picture2 lcd_display(network_client.localIP);
    libusb_async_reader barcode_reader;

///========================== threadstarters sig-slot connect =========================================
    QObject::connect(&controller_thread, &QThread::started, &_controller,    &controller::start);
    QObject::connect(&net_thread,        &QThread::started, &network_client, &network::start);
    //QObject::connect(&picture_thread,    &QThread::started, &lcd_display,    &picture2::start);

///========================== OBJECTS move to thread =========================================
    _controller.moveToThread(&controller_thread);
    network_client.moveToThread(&net_thread);
    //lcd_display.moveToThread(&picture_thread);

///========================== OBJECTS sig-slot connect =========================================
    QObject::connect( &barcode_reader, &libusb_async_reader::readyRead,     &_controller,    &controller::send_barcode );
    QObject::connect( &_controller,    SIGNAL(setPictureSIG(int, QString)), &lcd_display,    SLOT(setPicture(int, QString)));
    QObject::connect( &network_client, &network::log,                       &lcd_display,    &picture2::log);
    QObject::connect( &network_client, &network::network_ready,             &_controller,    &controller::ext_provided_network_ready);
    QObject::connect( &network_client, &network::enter_STATE_server_search_signal,      &_controller,    &controller::ext_provided_server_search);
    QObject::connect( &network_client, &network::readyRead,                 &_controller,    &controller::new_cmd_parse);
    QObject::connect( &_controller,    &controller::send_to_server,         &network_client, &network::SendToServer);

///========================== OBJECTS threads start =========================================
    controller_thread.start(QThread::TimeCriticalPriority);
    net_thread.start(QThread::TimeCriticalPriority);
    //picture_thread.start(QThread::TimeCriticalPriority);
    lcd_display.start();


    //===================== NFC =======================
    //NFC nfc("/dev/ttyAPP0");
    //QObject::connect(&nfc, SIGNAL(toPass(QByteArray)), &_controller, SLOT(send_barcode(QByteArray)));
    //nfc.open();
    //nfc.wakeup();
    //QThread::msleep(500);
   //quint32 versiondata = nfc.get_version();
   //nfc.get_status();
    //
    //nfc.SAMConfig();
    //
    //nfc.InAutoPoll();

    //QObject::connect(&_controller, &controller::exit, [&a](){qDebug() << "hop";//a.exit(42);
    //a.closeAllWindows();}   );

    return a.exec();
}
