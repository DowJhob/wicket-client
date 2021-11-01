#include "network.h"

network::network(int reconnect_interval, int timeout_interval):
    reconnect_interval(reconnect_interval), timeout_interval(timeout_interval)
{

}

void network::start()
{
    reconnect_timeout_timer = new QTimer(this);
    tcpSocket = new QTcpSocket(this);
    //for test
    connect(tcpSocket, &QTcpSocket::disconnected, this, [this](){emit log("socket disconnect sig");});
    connect(reconnect_timeout_timer, &QTimer::timeout, this, [this](){emit log("reconnect_timer timeout sig\n");});
    //================================
    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_5_12);
    machine = new QStateMachine(this);
    serverSearch = new QState(machine);
    startTCPconnection = new QState(machine);
    TCPconnected = new QState(machine);

    //========================================== search host ========================================
    machine->setInitialState( serverSearch );

    serverSearch->addTransition(      reconnect_timeout_timer, &QTimer::timeout,          serverSearch);    // если за время реконнекта не найдем сервер заново установим начальное состояние
    serverSearch->addTransition(                         this, &network::TCPserver_found, startTCPconnection); // если бродкастом нашли переходим к попытке соединения
    startTCPconnection->addTransition(reconnect_timeout_timer, &QTimer::timeout,          serverSearch);    // если тут залипнем то по таймеру начнем сначала
    startTCPconnection->addTransition(              tcpSocket, &QTcpSocket::connected,    TCPconnected);       // если соединились то запускаем таймер пересоединения в слоте
    TCPconnected->addTransition(                    tcpSocket, &QTcpSocket::disconnected, serverSearch);    // тут все понятно, если дисконеткт сокета то снова ищем
    TCPconnected->addTransition(      reconnect_timeout_timer, &QTimer::timeout,          serverSearch);    // но сигнала дисконект не будет если просто вынуть провод
    //---------------------
    connect(serverSearch,       &QState::entered, this, &network::server_search);
    connect(startTCPconnection, &QState::entered, this, &network::reconnect);

    connect(TCPconnected, &QState::entered, this, [this](){ reconnect_timeout_timer->setInterval(timeout_interval);
        reconnect_timeout_timer->start();
        localIP = tcpSocket->localAddress().toString();
        network_status = net_state::tcp_connected;
        emit log("TCP connected:\n");
        SendToServer(message( MachineState::undef, command::get_Register, QVariant(MACAddress)));
        emit network_ready();});
    connect(TCPconnected, &QState::exited, this, [this](){ network_status = net_state::search;
        emit log("TCP disconnected:\n");});
    sockets_init();
    //========================================= timers setup ========================================
    machine->start();
}

void network::SendToServer(message in)
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

void network::set_reconnect_interval(int reconnect_interval)
{
    reconnect_timeout_timer->setInterval(reconnect_interval);
}

void network::get_interface()
{
    for(QNetworkInterface netInterface : QNetworkInterface::allInterfaces())
        for (QNetworkAddressEntry entry : netInterface.addressEntries())
            if ( (netInterface.flags().testFlag(QNetworkInterface::IsUp)) &&
                 (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) && (!entry.ip().isLoopback()) )
            {
                broadcast_addr = entry.broadcast();
                MACAddress = netInterface.hardwareAddress();
                localIP = entry.ip().toString();
                fprintf(stdout,"%s\n", ( netInterface.name() + "/" + localIP + "/" + entry.netmask().toString() + "/" + MACAddress).toStdString().c_str());
                fflush(stdout);
                //break;

                QByteArray datagram = "turnstile";
                udpSocket->writeDatagram(datagram, broadcast_addr, udpPort);
            }
}

void network::logger(QString log)
{
    SendToServer(message( MachineState::undef, command::heartbeat, log));
}

void network::sockets_init()
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

void network::server_search()
{
    char * data;
    reconnect_timeout_timer->setInterval(reconnect_interval);
    tcpSocket->abort();
    udpSocket->readDatagram( data, 0 );
    network_status = net_state::search;
    emit log("server search started:\n");
    get_interface();
    emit enter_STATE_server_search_signal();
    reconnect_timeout_timer->start();
}

void network::tcp_readyRead_slot()
{

    //      int time = reconnect_timer->remainingTime();
    //      if (time < 100)
    //           qDebug() << QString::number(time);
    reconnect_timeout_timer->start();
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

void network::slotError(QAbstractSocket::SocketError err)
{
    //   network_status = state::error;
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

void network::slot_stateChanged(QAbstractSocket::SocketState socketState)
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

void network::processPendingDatagrams()
{
    if(network_status == net_state::tcp_connected)
    {
        char * data;
        udpSocket->readDatagram( data, 0 );
        return;
    }
    QByteArray datagram;
    QHostAddress _ip_addr;
    while (udpSocket->hasPendingDatagrams()) {
        datagram.resize(int(udpSocket->pendingDatagramSize()));
        udpSocket->readDatagram(datagram.data(), datagram.size(), &_ip_addr);
        if ( datagram == "server_v2" )
        {
            emit log("recieved UDP datagramm: " + datagram + " from: " + _ip_addr.toString() + "\n");
            server_ip_addr = _ip_addr;
            emit TCPserver_found();
            break;
        }
    }
}

void network::reconnect()
{
    reconnect_timeout_timer->start();
    emit log("TCP reconnect:\n");
    //        connected = false;
    tcpSocket->connectToHost(server_ip_addr, control_tcp_port, QIODevice::ReadWrite);
    //        if ( ++attempt_count > max_attempt_count )
    //            emit network_unavailable();
}
