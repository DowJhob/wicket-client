#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <netinet/in.h>

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
    int network_status = state::undefined;
    QHostAddress server_ip_addr = QHostAddress("10.1.7.11");
    QString MACAddress;
    QString localIP;
    quint16 udpPort = 27006;
    quint16 control_tcp_port = 27005;
    quint16 file_transfer_port = 27008;
    bool iron_mode = false;
    command msg;
    network(int reconnect_interval = 20000): reconnect_interval(reconnect_interval)
    {

    }
    ~network()
    {
    }
public slots:
    void start()
    {
 //       reconnect_timer2 = new QTimer(this);
        reconnect_timer = new QTimer(this);
        tcpSocket = new QTcpSocket(this);

        //for test
        connect(tcpSocket, &QTcpSocket::disconnected, this, [=](){emit log("socket disconnect sig");});
        connect(reconnect_timer, &QTimer::timeout, this, [=](){emit log("reconnect_timer timeout sig");});
     //================================
        in.setDevice(tcpSocket);
        in.setVersion(QDataStream::Qt_5_12);
        machine = new QStateMachine(this);
        broadcast_server_search = new QState(machine);
        network_StartTCPconnection = new QState(machine);
        network_TCPconnected = new QState(machine);

        //========================================== search host ========================================
 //       machine->addState( broadcast_server_search );
 //       machine->addState( network_StartTCPconnection );
 //       machine->addState( network_TCPconnected );
        machine->setInitialState( broadcast_server_search );

        broadcast_server_search->addTransition(   reconnect_timer,     &QTimer::timeout,         broadcast_server_search);    // если за время реконнекта не найдем сервер заново установим начальное состояние
        broadcast_server_search->addTransition(              this,    &network::TCPserver_found, network_StartTCPconnection); // если бродкастом нашли переходим к попытке соединения
        network_StartTCPconnection->addTransition(reconnect_timer,     &QTimer::timeout,         broadcast_server_search);    // если тут залипнем то по таймеру начнем сначала
        network_StartTCPconnection->addTransition(      tcpSocket, &QTcpSocket::connected,       network_TCPconnected);       // если соединились то запускаем таймер пересоединения в слоте
        network_TCPconnected->addTransition(            tcpSocket, &QTcpSocket::disconnected,    broadcast_server_search);    // тут все понятно, если дисконеткт сокета то снова ищем
        network_TCPconnected->addTransition(      reconnect_timer,     &QTimer::timeout,         broadcast_server_search);    // но сигнала дисконект не будет если просто вынуть провод
        //---------------------
        connect(broadcast_server_search, &QState::entered, this, &network::server_search);
        connect(network_StartTCPconnection, &QState::entered, this, &network::reconnect);

        connect(network_TCPconnected, &QState::entered, this, [=](){reconnect_timer->start();
                                                                   localIP = tcpSocket->localAddress().toString();
                                                                   network_status = state::ready;
                                                                   emit log("TCP connected:\n");
                                                                   SendToServer(command(_type::command, _comm::_register, QVariant(MACAddress)));
                                                                   emit network_ready();});
        connect(network_TCPconnected, &QState::exited, this, [=](){network_status = state::disconnected;
                                                                   //        test_data = QString::number(reconnect_timer->remainingTime());
                                                                   emit log("TCP disconnected:\n");});
//connect(network_TCPconnected, &QState::entered, reconnect_timer2, &QTimer::stop);
        sockets_init();
        //========================================= timers setup ========================================
        reconnect_timer->setInterval(reconnect_interval);
//reconnect_timer2->setSingleShot(true);
//connect(reconnect_timer2, &QTimer::timeout, this, &network::server_search);
//        reconnect_timer2->setInterval(reconnect_interval);

        machine->start();
       // server_search();
      //  reconnect();
    }
    void SendToServer(command in)
    {
        if ( tcpSocket->state() != QAbstractSocket::ConnectedState )
            return; 
        QByteArray  arrBlock;
                QDataStream out(&arrBlock, QIODevice::WriteOnly);
                out.setVersion(QDataStream::Qt_5_12);
                out << quint16(0) << in;
                out.device()->seek(0);
                out << quint16(arrBlock.size() - sizeof(quint16));
  //      qDebug()<< "send to server: " << in << "--" << arrBlock.size() << arrBlock.toStdString().c_str();
        tcpSocket->write(arrBlock);
    }

    void set_reconnect_interval(int reconnect_interval)
    {
        reconnect_timer->setInterval(reconnect_interval);
    }

private slots:
    void sockets_init()
    {
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;
        udpSocket = new QUdpSocket(this);

        connect(udpSocket, &QUdpSocket::readyRead, this, &network::processPendingDatagrams);
        while ( !udpSocket->bind( QHostAddress::Any, udpPort ) )  //слушаю все
        {
            emit log("Server Error - Unable to start the UDP listener: " + udpSocket->errorString());
            udpSocket->close();
        }
        //========================================= tcpSocket setup ========================================
//        tcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
//        tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
 //       tcpSocket->setSocketOption(QAbstractSocket::TypeOfServiceOption, 160);
  //              setsockopt (tcpSocket->socketDescriptor(), SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout) );
   //             setsockopt (tcpSocket->socketDescriptor(), SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout) );
        connect(tcpSocket, &QTcpSocket::readyRead, this, &network::tcp_readyRead_slot);
  //      connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(slotError(QAbstractSocket::SocketError)));
//        connect(&tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT( slot_stateChanged(QAbstractSocket::SocketState)));
    }
    void server_search()
    {
        tcpSocket->abort();
        network_status = state::network_search_host;
        emit log("server search started:\n");
        foreach(QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
            foreach (QNetworkAddressEntry entry, netInterface.addressEntries())
                if ((entry.ip().protocol() == QAbstractSocket::IPv4Protocol) && (!entry.ip().isLoopback()))
                {
                    quint32 first_subnet_address = entry.ip().toIPv4Address() & entry.netmask().toIPv4Address();
                    quint32 count_subnet_address = ~entry.netmask().toIPv4Address();
                    broadcast_addr = QHostAddress(first_subnet_address + count_subnet_address);
                    MACAddress = netInterface.hardwareAddress();
                    localIP = entry.ip().toString();
                    fprintf(stdout,"%s\n", ( netInterface.name() + " / " + localIP + "/" + entry.netmask().toString() + "/" + MACAddress).toStdString().c_str());
                    fflush(stdout);
                    QByteArray datagram = "turnstile";
                    udpSocket->writeDatagram(datagram, broadcast_addr, udpPort);
                }
        emit enter_STATE_server_search_signal();
        reconnect_timer->start();
  //      reconnect_timer2->start();
    }


    void tcp_readyRead_slot()
    {

  //      int time = reconnect_timer->remainingTime();
  //      if (time < 100)
 //           qDebug() << QString::number(time);
        reconnect_timer->start();
        for (uint i = 0; i < 0xFFFF; i++)
        {
            if (!m_nNextBlockSize) {
                if (tcpSocket->bytesAvailable() < sizeof(quint16)) {
                    break;
                }
                in >> m_nNextBlockSize;
            }
            if (tcpSocket->bytesAvailable() < m_nNextBlockSize) {
                break;
            }
            in >> msg;
            m_nNextBlockSize = 0;
            emit readyRead(msg);
        }
    }
    void slotError(QAbstractSocket::SocketError err)
    {
        network_status = state::error;
        QString strError =
                (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                         "The remote host is closed." :
                         err == QAbstractSocket::ConnectionRefusedError ?
                             "The connection was refused." :
                             QString(tcpSocket->errorString())
                             );
//        test_data = "TCP client error: " + strError;
        emit log("TCP error: " + strError + "\n");
    }
    void slot_stateChanged(QAbstractSocket::SocketState socketState)
    {
    //    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

        QString strError =
                (socketState == QAbstractSocket::UnconnectedState ?
                     "not connected." :
                     socketState == QAbstractSocket::HostLookupState ?
                         "performing a host name lookup." :
                         socketState == QAbstractSocket::ConnectingState ?
                             "started establishing a connection." :
                             socketState == QAbstractSocket::ConnectedState?
                                 "connection is established." :
                                 socketState == QAbstractSocket::BoundState?
                                     "bound to an address and port." :
                                     socketState == QAbstractSocket::ClosingState?
                                         "about to close (data may still be waiting to be written)." :
                                         socketState == QAbstractSocket::ListeningState?
                                             "connection was refused.":
                                             "No errors??");
        emit log("TCP state changed: " + strError + "\n");
    }
    void processPendingDatagrams()
    {
        QByteArray datagram;
        QHostAddress _ip_addr;
        while (udpSocket->hasPendingDatagrams()) {
            datagram.resize(int(udpSocket->pendingDatagramSize()));
            udpSocket->readDatagram(datagram.data(), datagram.size(), &_ip_addr);
            if ( datagram == "server" )
            {
                emit log("recieved UDP datagramm: " + datagram + " from: " + _ip_addr.toString() + "\n");
                server_ip_addr = _ip_addr;
                emit TCPserver_found();
            }
        }
    }
    void reconnect()
    {
        reconnect_timer->start();
        emit log("TCP reconnect:\n");
        //        connected = false;
        tcpSocket->connectToHost(server_ip_addr, control_tcp_port, QIODevice::ReadWrite);
//        if ( ++attempt_count > max_attempt_count )
//            emit network_unavailable();
    }
private:
//    QString test_data{};

    QElapsedTimer t;
    QStateMachine *machine;
    QState *broadcast_server_search;
    QState *network_StartTCPconnection;
    QState *network_TCPconnected;
    struct timeval timeout;
    QHostAddress broadcast_addr;
    QUdpSocket *udpSocket;
    QTcpSocket *tcpSocket;
    quint16 m_nNextBlockSize = 0;
    QTimer *reconnect_timer;
QTimer *reconnect_timer2;
    int reconnect_interval;// = 3000;

    int attempt_count = 0;
    int max_attempt_count = 10;

    QDataStream in;

signals:
    void readyRead(command);
    void signal_reconnect();
    void network_ready();
    void network_unavailable();

    void enter_STATE_server_search_signal();
    void TCPserver_found();

    void log(QString);
};

#endif // NETWORK_H
