#include "network.h"

network::network(int reconnect_interval, int timeout_interval):
    reconnect_interval(reconnect_interval), timeout_interval(timeout_interval)
{

}

void network::start()
{
    reconnect_timer = new QTimer(this);
    tcpSocket = new QTcpSocket(this);

    connect(reconnect_timer, &QTimer::timeout, this, [this](){emit log("reconnect_timer timeout sig\n");});
    //================================
    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_5_12);
    machine = new QStateMachine(this);
    onServerLost = new QState(machine);
    onConnection = new QState(machine);
    onConnected = new QState(machine);

    //========================================== search host ========================================
    machine->setInitialState( onServerLost );

    onServerLost->addTransition(reconnect_timer, &QTimer::timeout,          onServerLost);    // если за время реконнекта не найдем сервер заново установим начальное состояние
    onServerLost->addTransition(           this, &network::serverFound,     onConnection); // если бродкастом нашли переходим к попытке соединения
    onConnection->addTransition(reconnect_timer, &QTimer::timeout,          onServerLost);    // если тут залипнем то по таймеру начнем сначала
    onConnection->addTransition(      tcpSocket, &QTcpSocket::connected,    onConnected);       // если соединились то запускаем таймер пересоединения в слоте
    onConnected->addTransition(       tcpSocket, &QTcpSocket::disconnected, onServerLost);    // тут все понятно, если дисконеткт сокета то снова ищем
    onConnected->addTransition( reconnect_timer, &QTimer::timeout,          onServerLost);    // но сигнала дисконект не будет если просто вынуть провод
    //---------------------
    connect(onServerLost, &QState::entered, this, &network::processing_onServerLost);
    connect(onServerLost, &QState::entered, this, &network::serverLost);
    connect(onConnection, &QState::entered, this, &network::serverConnection);

    connect(onConnected, &QState::entered, this, &network::serverReady);

    connect(onConnected, &QState::entered, this, [this]()
    {
        reconnect_timer->start(reconnect_interval);
        network_status = net_state::tcp_connected;
        emit log("TCP connected:\n");
        //SendToServer(message( MachineState::undef, command::onRegister, QVariant(MACAddress)));
    });
    connect(onConnected, &QState::exited, this, [this]()
    {
        tcpSocket->abort();
        emit log("TCP disconnected:\n");});

    sockets_init();

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
    reconnect_timer->setInterval(reconnect_interval);
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

void network::processing_onServerLost()
{
    network_status = net_state::search;
    reconnect_timer->start(reconnect_interval);
    tcpSocket->abort();
    char * data;
    udpSocket->readDatagram( data, 0 );
    emit log("server lost:\n");
    get_interface();
    //emit serverLost();
}

void network::tcp_readyRead_slot()
{
    reconnect_timer->start(reconnect_interval);
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

        if (msg.cmd == command::getMAC)
            SendToServer(message( MachineState::undef, command::onRegister, QVariant(MACAddress)));
        else
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
    char * data;
    if(network_status == net_state::tcp_connected)
    {
        udpSocket->readDatagram( data, 0 );
        return;
    }
    QByteArray datagram;
    QHostAddress _ip_addr;
    while (udpSocket->hasPendingDatagrams())
    {
        int size = int(udpSocket->pendingDatagramSize());
        datagram.resize(size);
        udpSocket->readDatagram(datagram.data(), datagram.size(), &_ip_addr);

        datagramBuffer.append(datagram);
        if ( datagramBuffer.contains("server_v3" ))
        {
            emit log("recieved UDP datagramm: " + datagram + " from: " + _ip_addr.toString() + "\n");
            server_ip_addr = _ip_addr;
            emit serverFound();
            udpSocket->readDatagram( data, 0 );
            datagramBuffer.clear();
            break;
        }
        if (datagramBuffer.size() > 0xffff)
            datagramBuffer.clear();
    }
}

void network::serverConnection()
{
    reconnect_timer->start(reconnect_interval);
    emit log("TCP reconnect:\n");
    tcpSocket->connectToHost(server_ip_addr, control_tcp_port, QIODevice::ReadWrite);
}
