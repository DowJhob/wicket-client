#ifndef TCP_SERVER2_H
#define TCP_SERVER2_H
//#include <wicket_object.h>
#include <common_types.h>

#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QDataStream>
#include <QDebug>
#include <QObject>
#include <QTimer>

#include <QNetworkAccessManager>
#include <QNetworkInterface>

class network : public QObject {
    Q_OBJECT
public:
    bool iron_mode_t = false;
    int numConnections = 60;

    quint16 tcp_widgetPort = 27008;
    QHostAddress broadcast_addr = QHostAddress("10.1.7.255");
    quint16 broadcast_port = 27006;
    int reconnect_time = 1000;

    network(quint16 tcp_widgetPort = 27008, QObject* pwgt = nullptr) : QObject (pwgt)
    {
        //=============== CLIENT UDP LISTENER =============================================
        connect(&udpSocket, SIGNAL(readyRead()), this, SLOT(slot_UDP_readyRead()) );
        while ( !udpSocket.bind( QHostAddress::AnyIPv4, broadcast_port ) )  //слушаю все https://ru.stackoverflow.com/questions/792489/qt-qudpsocket-%D0%BF%D1%80%D0%B8%D0%B5%D0%BC-%D0%B4%D0%B5%D0%B9%D1%82%D0%B0%D0%B3%D1%80%D0%B0%D0%BC%D0%BC%D1%8B-broadcast
        {
            emit logger("Server Error - Unable to start the UDP listener: " + udpSocket.errorString());
            udpSocket.close();
        }
        connect(&socket, SIGNAL(readyRead()), this, SLOT(slot_tcp_read()) );
        connect(&socket, SIGNAL(connected()), this, SLOT( connected()));
        connect(&socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT( slot_stateChanged(QAbstractSocket::SocketState)));
        reconnect_timer.setInterval(reconnect_time);
        connect(&reconnect_timer, SIGNAL(timeout()), this, SLOT( send_wicket_service_search_request()));
        reconnect_timer.start();
        send_wicket_service_search_request();
    }
    ~network()
    {

    }
public slots:
    void send_to_socket( const QString& str)
    {
        QByteArray  arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_12);
        out << quint16(0) << str;
        out.device()->seek(0);
        out << quint16(arrBlock.size() - sizeof(quint16));
        socket.write(arrBlock);
    }

private:

    bool connection_flag = false; //что бы только один интерфейс подключался
    QTcpSocket socket;
    QUdpSocket udpSocket;
    quint16 m_nNextBlockSize = 0;
    QString str = "";
    quint16 CRC;
    QTimer reconnect_timer;
private slots:
    void send_wicket_service_search_request()
    {
        connection_flag = false;
        foreach(QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
            foreach (QNetworkAddressEntry entry, netInterface.addressEntries())
                if ((entry.ip().protocol() == QAbstractSocket::IPv4Protocol) //&& (!entry.ip().isLoopback())
                        )
                {
                    quint32 first_subnet_address = entry.ip().toIPv4Address() & entry.netmask().toIPv4Address();
                    quint32 count_subnet_address = ~entry.netmask().toIPv4Address();
                    broadcast_addr = QHostAddress(first_subnet_address + count_subnet_address);

                    udpSocket.writeDatagram(QByteArray("widget_client"), broadcast_addr, broadcast_port);
                }
    }

    void slot_UDP_readyRead()
    {
        QByteArray datagramm;
        QHostAddress _ip_addr(0xFFFFFFFF);
        while ( udpSocket.hasPendingDatagrams() )
        {
            datagramm.resize(int(udpSocket.pendingDatagramSize()));
            udpSocket.readDatagram(datagramm.data(), datagramm.size(), &_ip_addr, &broadcast_port);
            if (connection_flag)
                return;
            if ( datagramm == "server" )
            {
                emit logger("recive UDP datagramm: " + datagramm + " / from: " + _ip_addr.toString());
                connection_flag = true;
                socket.abort();
                socket.connectToHost(_ip_addr, tcp_widgetPort, QIODevice::ReadWrite);
            }
        }
    }
    void connected()
    {
        reconnect_timer.start();
        send_to_socket("refresh");
        emit logger("connected to wicket service ");

    }
    void slot_tcp_read()
    {
        reconnect_timer.start();
        QDataStream in(&socket);
        in.setVersion( QDataStream::Qt_5_12 );
        for (;;) {
            if (!m_nNextBlockSize) {
                if (socket.bytesAvailable() < sizeof(quint16))
                    break;
                in >> m_nNextBlockSize;
            }
            if (socket.bytesAvailable() < m_nNextBlockSize)
                break;
            in >> str;
            m_nNextBlockSize = 0;
            emit readyRead(str);
        }

    }

    void slot_tcp_Error(QAbstractSocket::SocketError socketError)
    {
        QString strError =
                "Error: " + (socketError == QAbstractSocket::HostNotFoundError ?
                                 "The host was not found." :
                                 socketError == QAbstractSocket::RemoteHostClosedError ?
                                     "The remote host is closed." :
                                     socketError == QAbstractSocket::ConnectionRefusedError ?
                                         "The connection was refused." :
                                         socket.errorString()
                                         );
        qDebug() << " - " << strError;
    }
    void slot_stateChanged(QAbstractSocket::SocketState socketState)
    {
        //QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

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
        emit logger("TCP state changed: " + strError + "\n");
        //qDebug() << "TCP state changed: " + strError + "\n";
    }
    void disconnect_socket_slot()
    {
        connection_flag = false;
        send_wicket_service_search_request();
        emit logger("<span style=""background-color:#d06060;""> service socket - " + socket.peerAddress().toString() + " disconnect</span>" );

    }

signals:
    void readyRead( QString );
    void logger( QString );

};

#endif // TCP_SERVER_H

