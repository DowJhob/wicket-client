#ifndef NIKIRET_2_H
#define NIKIRET_2_H

#include <stdio.h>
//#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

//#include <QObject>
//#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QQueue>
#include <QSerialPort>
//#include <QSocketNotifier>
//#include <QStateMachine>

#include <common_types.h>

//#define NFC_ON

class nikiret: public QObject {
    Q_OBJECT
public:
    QString output_str;
    int state = state::undefined;
    int pred_state = state::undefined;
    dir_type _direction;
    bool stop_polling = false;
    QString cross_brd_temp;
    QString error_string;
    nikiret(QString serial_port_name = "/dev/ttyAPP3");
    ~nikiret();
private:
    QSerialPort serialPort;
    QString serial_port_name;
    int pulse_width = 500;
    int state_polling_time = 1000;
    QByteArrayList init_list {"WAKE\r",
                      #ifdef NFC_ON
                              "PHER_ON\r",
                      #endif
                              "WDT_ON\r",
                              "DV_DIS\r",
                              "LIGHT_ON\r",
                              "SPIN1=1\r",
                              "SPIN2=1\r",
                              "SPIN3=1\r",
                              "PIN1_OFF\r",
                              "PIN2_OFF\r",
                              "PIN3_OFF\r",
                              "PIN4_EN\r",
                              "PIN5_EN\r",
                              //"PIN6_EN\r",
                              "TL_OFF\r",
                              "STATE\r"
                             };
    QByteArray buff{};
    QTimer *pass_sequence_timer;
    QTimer *lock_unlock_timer_sequence;
    QTimer *state_polling_timer;

    QTimer *flash_timer;
    bool greenLstate = false;
    bool redLstate = false;

    char heater = 'D';
public slots:
    void start();
    void init();
    void open_port();

    void setSerialPortName(QString serial_port_name);
    void setGREEN();
    void setRED();
    void setLightOFF();
    void setFLASH(color color);
    void stopFLASH();

    void alarm();
    void set_turnstile_to_pass(dir_type direction);
    void lock_unlock_sequence();

private slots:
    void slot_pass_sequnce_timer();
    void slot_lock_unlock_sequnce_timer();
    void slot_readyRead();
    void slot_state_polling();
    void slot_flash();
private:
    typedef void (nikiret::*MyCoolMethod)();
    void (nikiret::*flasher)() = &nikiret::setGREEN;

    void send_to_crossboard(QByteArray command);

    void recieve_buff_parse();
    void RDY_IO_processing(int turnstile_state);
    void termostatProcessing(int temp_pos);
signals:
    void armed();
    void unlocked();
    void passed();
    void error();
    void temp(QString);
};

#endif // NIKIRET_H
