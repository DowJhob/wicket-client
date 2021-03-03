#ifndef NFC_COPY_H
#define NFC_COPY_H

#include <stdio.h>
//#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include <QQueue>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QSerialPort>

#include <common_types.h>

#define PN532_POSTAMBLE               0x00
#define PN532_HOSTTOPN532             (0xD4)
#define PN532_PN532TOHOST             (0xD5)

// PN532 Commands
#define PN532_COMMAND_DIAGNOSE              (0x00)
#define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
#define PN532_COMMAND_GETGENERALSTATUS      (0x04)
#define PN532_COMMAND_READREGISTER          (0x06)
#define PN532_COMMAND_WRITEREGISTER         (0x08)
#define PN532_COMMAND_READGPIO              (0x0C)
#define PN532_COMMAND_WRITEGPIO             (0x0E)
#define PN532_COMMAND_SETSERIALBAUDRATE     (0x10)
#define PN532_COMMAND_SETPARAMETERS         (0x12)
#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_COMMAND_POWERDOWN             (0x16)
#define PN532_COMMAND_RFCONFIGURATION       (0x32)
#define PN532_COMMAND_RFREGULATIONTEST      (0x58)
#define PN532_COMMAND_INJUMPFORDEP          (0x56)
#define PN532_COMMAND_INJUMPFORPSL          (0x46)
#define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
#define PN532_COMMAND_INATR                 (0x50)
#define PN532_COMMAND_INPSL                 (0x4E)
#define PN532_COMMAND_INDATAEXCHANGE        (0x40)
#define PN532_COMMAND_INCOMMUNICATETHRU     (0x42)
#define PN532_COMMAND_INDESELECT            (0x44)
#define PN532_COMMAND_INRELEASE             (0x52)
#define PN532_COMMAND_INSELECT              (0x54)
#define PN532_COMMAND_INAUTOPOLL            (0x60)
#define PN532_COMMAND_TGINITASTARGET        (0x8C)
#define PN532_COMMAND_TGSETGENERALBYTES     (0x92)
#define PN532_COMMAND_TGGETDATA             (0x86)
#define PN532_COMMAND_TGSETDATA             (0x8E)
#define PN532_COMMAND_TGSETMETADATA         (0x94)
#define PN532_COMMAND_TGGETINITIATORCOMMAND (0x88)
#define PN532_COMMAND_TGRESPONSETOINITIATOR (0x90)
#define PN532_COMMAND_TGGETTARGETSTATUS     (0x8A)

#define PN532_RESPONSE_INDATAEXCHANGE       (0x41)
#define PN532_RESPONSE_INLISTPASSIVETARGET  (0x4B)


#define PN532_MIFARE_ISO14443A              (0x00)

// Mifare Commands
#define MIFARE_CMD_AUTH_A                   (0x60)
#define MIFARE_CMD_AUTH_B                   (0x61)
#define MIFARE_CMD_READ                     (0x30)
#define MIFARE_CMD_WRITE                    (0xA0)
#define MIFARE_CMD_WRITE_ULTRALIGHT         (0xA2)
#define MIFARE_CMD_TRANSFER                 (0xB0)
#define MIFARE_CMD_DECREMENT                (0xC0)
#define MIFARE_CMD_INCREMENT                (0xC1)
#define MIFARE_CMD_STORE                    (0xC2)

// FeliCa Commands
#define FELICA_CMD_POLLING                  (0x00)
#define FELICA_CMD_REQUEST_SERVICE          (0x02)
#define FELICA_CMD_REQUEST_RESPONSE         (0x04)
#define FELICA_CMD_READ_WITHOUT_ENCRYPTION  (0x06)
#define FELICA_CMD_WRITE_WITHOUT_ENCRYPTION (0x08)
#define FELICA_CMD_REQUEST_SYSTEM_CODE      (0x0C)

// Prefixes for NDEF Records (to identify record type)
#define NDEF_URIPREFIX_NONE                 (0x00)
#define NDEF_URIPREFIX_HTTP_WWWDOT          (0x01)
#define NDEF_URIPREFIX_HTTPS_WWWDOT         (0x02)
#define NDEF_URIPREFIX_HTTP                 (0x03)
#define NDEF_URIPREFIX_HTTPS                (0x04)
#define NDEF_URIPREFIX_TEL                  (0x05)
#define NDEF_URIPREFIX_MAILTO               (0x06)
#define NDEF_URIPREFIX_FTP_ANONAT           (0x07)
#define NDEF_URIPREFIX_FTP_FTPDOT           (0x08)
#define NDEF_URIPREFIX_FTPS                 (0x09)
#define NDEF_URIPREFIX_SFTP                 (0x0A)
#define NDEF_URIPREFIX_SMB                  (0x0B)
#define NDEF_URIPREFIX_NFS                  (0x0C)
#define NDEF_URIPREFIX_FTP                  (0x0D)
#define NDEF_URIPREFIX_DAV                  (0x0E)
#define NDEF_URIPREFIX_NEWS                 (0x0F)
#define NDEF_URIPREFIX_TELNET               (0x10)
#define NDEF_URIPREFIX_IMAP                 (0x11)
#define NDEF_URIPREFIX_RTSP                 (0x12)
#define NDEF_URIPREFIX_URN                  (0x13)
#define NDEF_URIPREFIX_POP                  (0x14)
#define NDEF_URIPREFIX_SIP                  (0x15)
#define NDEF_URIPREFIX_SIPS                 (0x16)
#define NDEF_URIPREFIX_TFTP                 (0x17)
#define NDEF_URIPREFIX_BTSPP                (0x18)
#define NDEF_URIPREFIX_BTL2CAP              (0x19)
#define NDEF_URIPREFIX_BTGOEP               (0x1A)
#define NDEF_URIPREFIX_TCPOBEX              (0x1B)
#define NDEF_URIPREFIX_IRDAOBEX             (0x1C)
#define NDEF_URIPREFIX_FILE                 (0x1D)
#define NDEF_URIPREFIX_URN_EPC_ID           (0x1E)
#define NDEF_URIPREFIX_URN_EPC_TAG          (0x1F)
#define NDEF_URIPREFIX_URN_EPC_PAT          (0x20)
#define NDEF_URIPREFIX_URN_EPC_RAW          (0x21)
#define NDEF_URIPREFIX_URN_EPC              (0x22)
#define NDEF_URIPREFIX_URN_NFC              (0x23)

#define PN532_GPIO_VALIDATIONBIT            (0x80)
#define PN532_GPIO_P30                      (0)
#define PN532_GPIO_P31                      (1)
#define PN532_GPIO_P32                      (2)
#define PN532_GPIO_P33                      (3)
#define PN532_GPIO_P34                      (4)
#define PN532_GPIO_P35                      (5)

// FeliCa consts
#define FELICA_READ_MAX_SERVICE_NUM         16
#define FELICA_READ_MAX_BLOCK_NUM           12 // for typical FeliCa card
#define FELICA_WRITE_MAX_SERVICE_NUM        16
#define FELICA_WRITE_MAX_BLOCK_NUM          10 // for typical FeliCa card
#define FELICA_REQ_SERVICE_MAX_NODE_NUM     32


class NFC: public QObject {
    Q_OBJECT
public:
    QString error_string;
    NFC(QString serial_port_name = "/dev/ttyAPP0"):serial_port_name(serial_port_name){}
    ~NFC(){}
    QByteArray PREAMBLE_STARTCODE = QByteArray::fromHex("0000FF");
    QByteArray ACK = QByteArray::fromHex("00FF00");
    QByteArray NACK = QByteArray::fromHex("FF0000");
    QByteArray Err = QByteArray::fromHex("01FF00");
    QByteArray _get_version = QByteArray::fromHex("02");
    QByteArray _get_status = QByteArray::fromHex("04");
    QByteArray _SAMConfig = QByteArray::fromHex("14011401");
    QByteArray _inListPassiveTarget = QByteArray::fromHex("4A0100");
    QByteArray _InAutoPoll = QByteArray::fromHex("60"  //PollNr specifies the number of polling (one polling is a polling for each Type j).
                                                 "FF"  //0x01 – 0xFE : 1 up to 254 polling
                                                 //0xFF : Endless polling.
                                                 "0F"  //Period (0x01 – 0x0F) indicates the polling period in units of 150 ms,
                                                 "00"//Type 1 indicates the mandatory target type to be polled at the 1st time,
                                                 "01"//Type 2 to Type N indicate the optional target types to be polled at the 2nd up to
                                                 "02"//the Nth time (N ≤ 15).
                                                 "03"//The format used for these fields is:
                                                 "04"
                                                 "10"
                                                 "11"
                                                 "12"
                                                 "20"
                                                 "23"
                                                 "40"
                                                 "41"
                                                 "428081");

    void wakeup()
    {
        qDebug() << "send wakeup";
        serialPort->write(QByteArray::fromHex("5555000000"));
    }
    void get_version()
    {
        qDebug() << "send get_version";
        writeCommand2222(&_get_version);
    }
    void get_status()
    {
        qDebug() << "send get_status";
        writeCommand2222(&_get_status);
    }
    void SAMConfig()
    {
        qDebug() << "send SAMConfig";
        writeCommand2222(&_SAMConfig);
    }
    void inListPassiveTarget()
    {
        qDebug() << "send inListPassiveTarget";
        writeCommand2222(&_inListPassiveTarget);
        if (serialPort->waitForReadyRead(-1))
        {
            qDebug() << "hop inlist";
            slot_readyRead();
        }
    }
    void InAutoPoll ()
    {
        qDebug() << "send InAutoPoll";
        writeCommand2222(&_InAutoPoll);
qDebug() << "exit InAutoPoll";
    }
    /**************************************************************************/
    /*!
        @brief  Exchanges an APDU with the currently inlisted peer

        @param  send            Pointer to data to send
        @param  sendLength      Length of the data to send
        @param  response        Pointer to response data
        @param  responseLength  Pointer to the response data length
    */
    /**************************************************************************/
    void inDataExchange(uint8_t *send, uint8_t sendLength, uint8_t *response, uint8_t *responseLength)
    {
        qDebug() << "send inDataExchange";
        //        uint8_t i;
        //      char inDataExchange[2]{0x40, // PN532_COMMAND_INDATAEXCHANGE;
        //              ,inListedTag};
        QByteArray inDataExchange;
        inDataExchange.append(0x40); // PN532_COMMAND_INDATAEXCHANGE;
        inDataExchange.append(inListedTag);
        inDataExchange += *send;
        writeCommand2222(&inDataExchange);
        // return ;
    }
private:
    QTimer _timeout;
    QTimer ACKtimeout;

    int timeout = 200;

    QSerialPort *serialPort;
    QString serial_port_name;
    QByteArray readDATA{};
    QByteArray payload{};
    uint8_t inListedTag; // Tg number of inlisted tag.
    QByteArray buff_last_command;
    QByteArray last_command;
    void recieve()
    {
        if (serialPort->waitForReadyRead(timeout))
        {
            slot_readyRead();
            return;
        }
    }

    void writeCommand2222(QByteArray *data_in)
    {
        while (true)
        {
            writeCommand(data_in);
            if (serialPort->waitForReadyRead(timeout))  //wait for ACK
            {
                slot_readyRead();
                break;
            }
        }
        if (serialPort->waitForReadyRead(-1))    //wait for payload
        {
            slot_readyRead();
            return;
        }
    }

    void writeCommand(QByteArray *data_in)
    {
        uint8_t hlen = data_in->length();
        buff_last_command.clear();
        buff_last_command.append(PREAMBLE_STARTCODE);
        uint8_t length = hlen + 1;   // length of data field: TFI + DATA
        buff_last_command += length;
        buff_last_command += (~length + 1);         // checksum of length
        buff_last_command += PN532_HOSTTOPN532;
        uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA
        buff_last_command.append(*data_in);
        for (uint8_t i = 0; i < hlen; i++) {
            sum += data_in->at(i);
        }
        uint8_t checksum = ~sum + 1;            // checksum of TFI + DATA
        buff_last_command += checksum;
        buff_last_command += (char)PN532_POSTAMBLE;
        //       qDebug() << "command_WRITE: " << lost_command.toHex('x');
        qint64 y = serialPort->write(buff_last_command);
        //        ACKtimeout.start(100);  //15 from datasheet
        //command_timeout.start(350);
    }
    void processed_payload(int TFI)
    {
        //        qDebug() << "check payload " << payload.toHex('x');
        if ( !payload.isEmpty())
            switch ( TFI )
            {
            case 0x03: qDebug() << "get fimware version: ";
                qDebug() << "Chip PN5" + QString::number(payload.at(0), 16);
                qDebug() << "Firmware ver. " + QString::number(payload.at(1)) + '.' + QString::number(payload.at(2));
                qDebug() << "Support. " + QString::number(payload.at(3));
                //payload.clear();
                break;
            case 0x05: qDebug() << "response GetGeneralStatus: Output  " << payload.toHex('x');
                break;
            case 0x07: qDebug() << "response ReadRegister: " << payload.toHex('x');
                break;
            case 0x09: qDebug() << "response WriteRegister: " << payload.toHex('x');
                break;
            case 0x15: qDebug() << "response SAMconfig: ";
                break;
            case 0x4B:
                if (buff_last_command.length() >= 9)
                    inListPassiveTarget_handler(buff_last_command.at(8));
                break;
            case 0x61: autopoll_handler(payload.at(1)); break;
            }
    }
    void autopoll_handler(int type)
    {
        switch (type)
        {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x20:
        case 0x23:payload = payload.mid(4); inListPassiveTarget_handler(0x0F & type);break;
        case 0x40:break;
        case 0x41:break;
        case 0x42:break;
        case 0x80:break;
        case 0x81:break;
        case 0x82:break;

        }

        qDebug() << "autopoll " << payload.toHex('x');
        emit toPass("9780201379624");
    }
    void inListPassiveTarget_handler(int type)
    {
        int lenght = payload.length();
        switch (type) {
        case 0x00:
            if ( lenght >= 6 && lenght >= (payload.at(4) + 5) )
            {
                qDebug() << "UID from NFC, 106 kbps type A: " + payload.mid(5, payload.at(4))//.toHex('|')
                            ;
                emit toPass(payload.mid(5, payload.at(4)));
            }
            break;
        case 0x01:///fallthrough!!!!!!!!!!!
        case 0x02:
            if ( lenght >= 11 )
            {
                qDebug() << "UID from NFC, (212 kbps or 424 kbps): " + payload.mid(3, 8)//.toHex('|')
                            ;
                emit toPass( payload.mid(3, 8));
            }
            break;
        case 0x03:
            qDebug() << "UID from NFC, 106 kbps Type B" + payload//.toHex('|')
                        ;
            emit toPass( payload.toHex('|'));
            break;
        case 0x04:
            if ( lenght >= 7 )
            {
                qDebug() << "UID from NFC: " + payload.mid(3, 4)//.toHex('|')
                            ;
                emit toPass(payload.mid(3, 4));
            }
            break;
        }
        writeCommand2222(&_InAutoPoll);
        qDebug() << "inListPassiveTarget_handler def: " << payload.toHex('x');

        emit toPass("9780201379624");
    }

    bool checkAmble()
    {
//        if( serialPort->bytesAvailable() < 6 )
//            return false;
//        readDATA = serialPort->read(6);
//        if (checkAmble())
//            return false;

        if ( readDATA.at(0)==0x00&&readDATA.at(1)==0x00&&readDATA.at(2)==0xFF)
        {
            if ( readDATA.at(3)==0x00&&readDATA.at(4)==0xFF&&readDATA.at(5)==0x00 )
            {
                qDebug() << "ACK: ";
                readDATA.clear();
                last_command.clear();
                return true;
            }
            if ( readDATA.at(3)==0xFF&&readDATA.at(4)==0x00&&readDATA.at(5)==0x00 )
            {
                qDebug() << "NAK";
                readDATA.clear();
                return true;
            }
            if ( readDATA.at(3)==0x01&&readDATA.at(4) == 0xFF )
            {
                qDebug() << "Error frame";
                readDATA.clear();
                return true;
            }
        }
        return false;
    }

public slots:
    void open()
    {
        serialPort = new QSerialPort(serial_port_name);
        serialPort->setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
        if(!serialPort->open(QIODevice::ReadWrite))
        {
            error_string = "NFC error: " + QString::number(serialPort->error());
            fprintf(stdout, "NFC turnstile module can't open device %s / error: %s\n", serial_port_name.toStdString().c_str(), error_string.toStdString().c_str());
        }
        else
        {
            fprintf(stdout, "NFC module open device: %s\n", serial_port_name.toStdString().c_str());
            error_string = "no error";
            //           connect(&serialPort, &QSerialPort::readyRead, this, &NFC::slot_readyRead);
        }
        fflush(stdout);
        wakeup();
        QThread::msleep(500);
        get_version();
        get_status();
        SAMConfig();
        //InAutoPoll();
        inListPassiveTarget();
    }

private slots:
//    void checkAmble()
//    {

//    }
    void slot_readyRead()
    {
        if( serialPort->bytesAvailable() < 6 )
            return;
        readDATA = serialPort->read(6);
        if (checkAmble())
            return;

        int message_lenght = readDATA.at( 3 )+1;

        if( serialPort->bytesAvailable() < message_lenght )
        {
            if (!serialPort->waitForReadyRead(timeout) /*|| serialPort->bytesAvailable() < message_lenght*/)
                return;
        }
        readDATA = serialPort->read(message_lenght);
        payload.clear();
        payload.append( readDATA.mid(1, message_lenght - 3)); //truncate all checksum, TFI and postamble
        qDebug() << "payload frame, TFI:" << "x"+QString::number(readDATA.at( 0 ), 16) << ", payload:" << payload.toHex('x').toUpper();
        serialPort->write(ACK);
        processed_payload(readDATA.at( 0 ));
        readDATA.clear();
    }

signals:
    void Log();
    void toPass(QByteArray);

};

#endif // NFC_COPY_H
