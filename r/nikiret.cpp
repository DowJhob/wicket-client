#include "nikiret.h"

nikiret::nikiret(QString serial_port_name):serial_port_name(serial_port_name)
{}

nikiret::~nikiret()
{
    //     serialPort.close();
}

void nikiret::start()
{
    open_port();
    //        pass_sequence_timer.setTimerType(Qt::PreciseTimer);
    pass_sequence_timer = new QTimer(this);
    pass_sequence_timer->setInterval(pulse_width);
    pass_sequence_timer->setSingleShot(true);
    connect(pass_sequence_timer, &QTimer::timeout, this, &nikiret::slot_pass_sequnce_timer);
    //        lock_unlock_timer_sequence.setTimerType(Qt::PreciseTimer);
    lock_unlock_timer_sequence = new QTimer(this);
    lock_unlock_timer_sequence->setInterval(pulse_width);
    lock_unlock_timer_sequence->setSingleShot(true);
    connect(lock_unlock_timer_sequence, &QTimer::timeout, this, &nikiret::slot_lock_unlock_sequnce_timer);
    //        state_polling_timer.setTimerType(Qt::PreciseTimer);
    state_polling_timer = new QTimer(this);
    connect(state_polling_timer, &QTimer::timeout, this, &nikiret::slot_state_polling);
    state_polling_timer->setInterval(state_polling_time);
    //        flash_timer.setTimerType(Qt::PreciseTimer);
    flash_timer = new QTimer(this);
    connect(flash_timer, &QTimer::timeout, this, &nikiret::slot_flash);
    flash_timer->setInterval(100);
    //flash_timer->start();
    init();
    //set_test();
}

void nikiret::init()
{
    for(QString s : init_list)
        send_to_crossboard(s.toLatin1());
    state_polling_timer->start();
}

void nikiret::open_port()
{
    //serialPort.close();
    serialPort.setPortName(serial_port_name);
    serialPort.setBaudRate(QSerialPort::Baud57600, QSerialPort::AllDirections);
    serialPort.setFlowControl(QSerialPort::NoFlowControl);
    if(!serialPort.open(QIODevice::ReadWrite))
    {
        error_string = "nikiret error: " + QString::number(serialPort.error());
        fprintf(stdout, "nikiret turnstile module can't open device %s / error: %s\n", serial_port_name.toStdString().c_str(), error_string.toStdString().c_str());
    }
    else
    {
        fprintf(stdout, "nikiret turnstile module open device %s\n", serial_port_name.toStdString().c_str());
        error_string = "no error";
        connect(&serialPort, &QSerialPort::readyRead, this, &nikiret::slot_readyRead);
    }
    fflush(stdout);
}

void nikiret::setSerialPortName(QString serial_port_name)
{
    this->serial_port_name = serial_port_name;
}

void nikiret::setGREEN()
{
    greenLstate = true;
    send_to_crossboard("GREEN\r");
}

void nikiret::setRED()
{
    redLstate = true;
    send_to_crossboard("RED\r");
}

void nikiret::setLightOFF()
{
    greenLstate = false;
    redLstate = false;
    send_to_crossboard("TL_OFF\r");
}

void nikiret::setFLASH(color color)
{
    switch (color)
    {
    case green: flasher = &nikiret::setGREEN;break;
    case red: flasher = &nikiret::setRED;break;
    }
}

void nikiret::stopFLASH()
{
    flash_timer->stop();
    setLightOFF();
}

void nikiret::alarm()
{
    send_to_crossboard("BEEP 100\r");
}

void nikiret::set_turnstile_to_pass(dir_type direction)
{
    _direction = direction;
    if (direction == dir_type::entry )
        send_to_crossboard("PIN1_ON\r");
    if (direction == dir_type::exit_ )
        send_to_crossboard("PIN2_ON\r");
    pass_sequence_timer->start();
}

void nikiret::lock_unlock_sequence()
{
    if ( state == state::ready )
        send_to_crossboard("PIN3_ON\rLIGHT_OFF\r");
    else
        send_to_crossboard("PIN3_ON\rLIGHT_ON\r");
    lock_unlock_timer_sequence->start();
}

void nikiret::slot_pass_sequnce_timer()
{
    if (_direction == dir_type::entry )
        send_to_crossboard("PIN1_OFF\r");
    if (_direction == dir_type::exit_)
        send_to_crossboard("PIN2_OFF\r");
}

void nikiret::slot_lock_unlock_sequnce_timer()
{
    send_to_crossboard("PIN3_OFF\r");
}

void nikiret::slot_readyRead()
{
    buff += serialPort.readAll();
    //qDebug() << buff;
    if ( buff.at(buff.length()-1) != '\r' )
        return;
    recieve_buff_parse();
}

void nikiret::slot_state_polling()
{
    //qDebug() << "cross_brd_temp: " << cross_brd_temp;
#ifdef TEST
    buff.append( "ALARM 0\rTEMP 24.5\rPIN4 1\rPIN5 0\rPIN6 0\rHEATER DISABLED\rPHER DISABLED\rVER 2.35\rHARDRST 0\r" );
    recieve_buff_parse();
#else
    if ( stop_polling )
        return;
    send_to_crossboard("STATE\r");
#endif
    emit temp(cross_brd_temp);
}

void nikiret::slot_flash()
{
    //      if (redLstate)
    //           setLightOFF();
    //       else
    //           setRED();
    if (greenLstate)
        setLightOFF();
    else
        (this->*flasher)();
}

void nikiret::send_to_crossboard(QByteArray command)
{
    serialPort.write(command);
    serialPort.flush();
}

void nikiret::recieve_buff_parse()
{
    //qDebug() <<  buff;
    if ( buff.indexOf("HARDRST 1") >= 0 )
    {
        fprintf(stdout, "hrdrst");
        fflush(stdout);
    }
    if ( buff.indexOf("PIN6 1") >= 0 )
        emit error();
    if ( buff.indexOf("PIN5 1") >= 0 )
        emit passed();
    if ( buff.indexOf("PIN4 1") >= 0 )
        RDY_IO_processing( state::ready );
    if ( buff.indexOf("PIN4 0") >= 0 )
        RDY_IO_processing( state::unlocked );
    int temp_pos = buff.indexOf("HEATER");
    if ( temp_pos >= 0 )
        heater = buff.at(temp_pos + 7);
    temp_pos = buff.indexOf("TEMP");
    if ( temp_pos >= 0 )
        termostatProcessing(temp_pos);
    buff.clear();
}

void nikiret::getState()
{
    if ( state == state::ready )
    {
        emit armed();
    }
    else if ( state == state::unlocked )
    {
        emit unlocked();
    }
}

void nikiret::RDY_IO_processing(int turnstile_state)
{
    pred_state = state;
    state = turnstile_state;
    if ( state != pred_state )
    {
        getState();
    }
}

void nikiret::termostatProcessing(int temp_pos)
{
    cross_brd_temp = buff.mid( temp_pos + 5, 4 );
    if( cross_brd_temp.mid( 0, 2 ).toInt() < 15 && heater == 'D')
        send_to_crossboard("HEATER_ON\r");
    else if( heater == 'E')
        send_to_crossboard("HEATER_OFF\r");
}
