#include <QApplication>
#include "../common/common_types.h"
#include "../common/widgets/mainStackedWgt.h"
#include "../common/network.h"
#include "controller.h"

//#define TEST
#ifdef TEST
#include "test_timer.h"
#include <NFC_copy.h>
#endif


#ifdef NFC_ON
#include <NFC_copy.h>
#endif


int main(int argc, char *argv[])
{    
    qDebug() << QString("Version: %1").arg(GIT_VERSION);
    qRegisterMetaType<message>("message");
    QApplication a(argc, argv, false);      //https://forum.qt.io/topic/2002/linux-non-gui-application-drawimage-crash/6
    //Constructs an application object with argc command line arguments in argv.
    //If GUIenabled is true, a GUI application is constructed, otherwise a non-GUI
    //(console) application is created. I mean this third QApplication parameter.
    //QThread::currentThread()->setPriority( QThread::TimeCriticalPriority );
    fprintf(stdout, "turnstile client - eulle@ya.ru\n");
    fflush(stdout);


    //serial serial;

    ///========================== OBJECTS =========================================
    controller _controller;
    network network_client(1500, 1000);
    ///========================== lcd_display =========================================
    mainStackedWgt lcd_display;
    //lcd_display.start();


    ///========================== controller =========================================
    QThread controller_thread;
    QObject::connect(&controller_thread, &QThread::started,      &_controller, &controller::start);
    _controller.moveToThread(&controller_thread);
    QObject::connect( &_controller, &controller::setCaption,   &lcd_display,    &mainStackedWgt::setCaption);
    QObject::connect( &_controller, &controller::s_showStatus,   &lcd_display,    &mainStackedWgt::showState);
    QObject::connect( &_controller, &controller::send_to_server, &network_client, &network::SendToServer);
    QObject::connect( &_controller, &controller::log,            &lcd_display,    &mainStackedWgt::log);
    controller_thread.start(//QThread::TimeCriticalPriority
                            );
    //_controller.start();
    ///========================== network =========================================
    QThread net_thread;
    QObject::connect(&net_thread,        &QThread::started, &network_client, &network::start);
    network_client.moveToThread(&net_thread);
    QObject::connect( &network_client, &network::log,         &lcd_display, &mainStackedWgt::log);
    QObject::connect( &network_client, &network::showState, &lcd_display, &mainStackedWgt::showState);
    //QObject::connect( &network_client, &network::serverReady, &_controller, &controller::serverReady);
    //QObject::connect( &network_client, &network::serverLost,  &_controller, &controller::serverLost);
    QObject::connect( &network_client, &network::readyRead,   &_controller, &controller::new_cmd_parse);
    net_thread.start(//QThread::TimeCriticalPriority
                     );
    //network_client.start();

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
