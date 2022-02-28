#include "controller.h"

controller::controller()
{
}

void controller::start()
{
    t = new QElapsedTimer;
    wicket_init();

    //createSNAPI();
    createSerial();

    ///-----------------------------------------------------
    //_updater.setPort( FILE_TRANSFER_PORT );
    //_updater.setFilename( QCoreApplication::applicationFilePath() );
    //connect(&_updater, &updater::ready, [ & ](){ emit exit(42); } );
    //==================================================================================================================
}

void controller::wicket_init()
{
    wicket = new nikiret();

    connect(wicket, &nikiret::temp,     this, [&](QString temp){emit send_to_server(message(MachineState::undef, command::onTemp, temp));});
    connect(wicket, &nikiret::armed,    this, [&](){emit send_to_server(message(MachineState::undef, command::onArmed));});
    connect(wicket, &nikiret::unlocked, this, [&](){emit send_to_server(message(MachineState::undef, command::onUnlock));});
    connect(wicket, &nikiret::passed,   this, [&](){emit send_to_server(message(MachineState::undef, command::onPassed));});

    wicket->start();
}

void controller::new_cmd_parse(message msg)
{
    cmd_arg = msg.body.toString();
    switch (msg.cmd) {
    //  безусловные команды
    case command::getState                 : wicket->getState();
        emit setCaption(cmd_arg);
        break;
    case command::setArmed                 : wicket->lock_unlock_sequence();    break;
    case command::setUnlock                : wicket->lock_unlock_sequence();    break;
    case command::setEntryOpen             : wicket->set_turnstile_to_pass(dir_type::entry); break;          // Открываем турникет
    case command::setExitOpen              : wicket->set_turnstile_to_pass(dir_type::exit_); break;            //

    case command::setGreenLampOn           : wicket->setGREEN(); break;
    case command::setRedLampOn             : wicket->setRED(); break;
    case command::setLampOff               : wicket->setLightOFF();  break;             // Отправляем команду погасить лампы

    case command::setAlarm                 : wicket->alarm(); break;              // Бибип


        // Показываем картинку с текстом на экране считывателя
    case command::showInfoStatus        :
    case command::showServiceStatus        :       // Турникет не готов и все такое
    case command::showPlaceTicketStatus          :         // Турникет готов, покажите билет или ковид куар
    case command::showOpenStatus           :          // Пжалста проходите, зелЁни стралачка

    case command::showPlaceCertStatus      :                   // синий? фон со стрелкой куда пихать
    case command::showCheckStatus      :                   // оранжевый фон с часиками
    case command::showFailStatus      : emit s_showStatus(msg); break;        // Красный крестик

    default :            break;
    }

}

void controller::createSNAPI()
{
    //QThread thread;
    //    uchar      EP_IN = 0x81;
    uint16_t     VID = 0x05E0;
    uint16_t     PID = 0x1900;
    int iface = 0;
    int config = 1;
    int alt_config = 0;
    barcodeReader = new snapi_barcode_reader(VID, PID, iface, config, alt_config);

    QObject::connect(barcodeReader, &snapi_barcode_reader::barcodeReady, this, &controller::local_barcode, Qt::QueuedConnection);
    //QObject::connect(barcode_reader, &snapi_barcode_reader::log,  &network_client, &network::logger, Qt::QueuedConnection);
    //QObject::connect(&thread, &QThread::started, barcode_reader, &libusb_async_reader::start);
    //barcode_reader->moveToThread(&thread);
    //thread.start(//QThread::TimeCriticalPriority
    //             );
    //barcode_reader->start();
}

void controller::createSerial()
{
    barcodeReader = new serial("/dev/ttyAMA0");
    QObject::connect(barcodeReader, &snapi_barcode_reader::barcodeReady, this, &controller::local_barcode, Qt::QueuedConnection);
}

void controller::local_barcode(QByteArray data)
{
    //t->start();
    emit send_to_server(message(MachineState::undef, command::onBarcode, data));
    qDebug() << "local_barcode: " + data;
}


