#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#ifdef unix
#include <netinet/in.h>
#endif

#include <QTcpSocket>
#include <QUdpSocket>
#include <QDataStream>
#include <QTimer>
#include <QNetworkInterface>
#include <QFile>
#include <QStateMachine>
#include <QElapsedTimer>
#include <QTime>


#include <common_types.h>
#include "command.h"


class network : public QObject
{
    Q_OBJECT
public:
    net_state network_status = net_state::undefined;
    QHostAddress server_ip_addr = QHostAddress("10.1.7.11");
    QString MACAddress;
    QString localIP;
    quint16 udpPort = 27006;
    quint16 control_tcp_port = 27005;
    quint16 file_transfer_port = 27008;
    bool iron_mode = false;
    message msg;
    network(int reconnect_interval = 2000, int timeout_interval = 20000);
public slots:
    void start();

    void SendToServer( message in);

    void set_reconnect_interval(int reconnect_interval);
    void get_interface();
    void logger(QString log);

private slots:
    void sockets_init();
    void server_search();
    void tcp_readyRead_slot();
    void slotError(QAbstractSocket::SocketError err);
    void slot_stateChanged(QAbstractSocket::SocketState socketState);
    void processPendingDatagrams();
    void reconnect();
private:
    QElapsedTimer t;
    QStateMachine *machine;
    QState *serverSearch;
    QState *startTCPconnection;
    QState *TCPconnected;
    struct timeval timeout;
    QHostAddress broadcast_addr;
    QUdpSocket *udpSocket;
    QTcpSocket *tcpSocket;
    quint16 m_nNextBlockSize = 0;
    QTimer *reconnect_timeout_timer;
    //QTimer *reconnect_timer2;
    int reconnect_interval;// = 3000;
    int timeout_interval;

    int attempt_count = 0;
    int max_attempt_count = 10;

    QDataStream in;

signals:
    void readyRead(message);

    void enter_STATE_server_search_signal();
    void TCPserver_found();
    void network_ready();

    void log(QString);
};

#endif // NETWORK_H
