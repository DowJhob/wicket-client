#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H
#include <QApplication>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDataStream>
#include <QFile>
#include <unistd.h>

class updater: public QObject
{
    Q_OBJECT
public:
    QString filename;
    QHostAddress server_addr;
    quint16 file_transfer_port;
    updater(QHostAddress server_addr = QHostAddress("10.1.7.10"), quint16 file_transfer_port = 27008, QString filename = "/r")
    {
        setFilename(filename);

        this->server_addr = server_addr;
        this->file_transfer_port = file_transfer_port;

        socket.setSocketOption(QAbstractSocket::KeepAliveOption, 1);
        socket.setSocketOption(QAbstractSocket::LowDelayOption, 1);
        socket.setSocketOption(QAbstractSocket::TypeOfServiceOption, 160);
        connect(&socket, SIGNAL(readyRead()), this, SLOT(pSocketReadyRead()));
        connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)));
        connect(&socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT( slot_stateChanged(QAbstractSocket::SocketState)));
        connect(&socket, SIGNAL( connected() ), this, SLOT( slotConnected() ) );
        connect(&socket, SIGNAL(disconnected()), this, SLOT(slot_disConnected()));

    }
    void request_update()
    {
        socket.connectToHost(server_addr, file_transfer_port, QIODevice::ReadWrite);
    }
    void setAddress(QHostAddress server_addr)
    {
        this->server_addr = server_addr;
    }
    void setFilename(QString filename)
    {
        this->filename = filename;
        QString s = filename + "-new";
        file.setFileName(s);
    }
    void setPort(quint16 file_transfer_port)
    {
        this->file_transfer_port = file_transfer_port;
    }

private slots:
    void slotConnected()
    {
        qDebug() << "socket connected";
        connected = true;
        SendToServer("ready for recive");
    }
    void pSocketReadyRead()
    {

        QDataStream stream(&socket);
        stream.setVersion(QDataStream::Qt_5_12);
        quint16 CRC;

        forever
        {
            if (nextBlockSize == 0)
            {
                if (socket.bytesAvailable() < sizeof(quint64))
                    break;
                stream >> nextBlockSize;
            }
            if (nextBlockSize > socket.bytesAvailable())
                break;

            QByteArray arrFile;
            stream >> arrFile;

            if (socket.bytesAvailable() < sizeof(quint16))
                break;
            stream >> CRC;

qDebug() << "nextBlockSize = " << nextBlockSize << " arrFile = " << arrFile.size() << "CRC = " << CRC;

            if ( CRC != qChecksum(arrFile, nextBlockSize) )
            {
                error_handler("file " + filename + " CRC not valid!");   // arrFile.size();
                return;
            }

            if ( !file.open(QIODevice::WriteOnly) )
            {
                qDebug() << "file " + file.fileName() + " not open";
                error_handler("file " + filename + " not open");   // arrFile.size();
                return;
            }
//file.seek(0);
            if ( file.write(arrFile, nextBlockSize) != nextBlockSize)
            {
                error_handler("file " + filename + " writed size not equal sended file");   // arrFile.size();
                return;
            }
            nextBlockSize = 0;

            if ( !file.flush() )
            {
                error_handler("file " + filename + " not flushed");   // arrFile.size();
                return;
            }
            if ( unlink(filename.toLatin1() ) ) /// << " (0 - OK, -1 - Fail"
            {
                error_handler("file " + filename + " not deleted (unlink fail)");
                return;
            }

            if ( !file.setPermissions( QFileDevice::ReadOwner	| QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadUser | QFileDevice::ExeUser) )
            {
                error_handler("file " + filename + " permission not set");
                return;
            }

            if ( !file.copy( filename.toLatin1() ) )
            {
                error_handler("file " + filename + " not renamed");
                return;
            }
            file.close();
            SendToServer("update success");
            emit ready();

        }
    }
    void slot_disConnected()
    {
        connected = false;
qDebug() << "socket disconnected";
    }
    void slotError(QAbstractSocket::SocketError err)
    {
        QString strError =
                (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                         "The remote host is closed." :
                         err == QAbstractSocket::ConnectionRefusedError ?
                             "The connection was refused." :
                             QString(socket.errorString())
                             );
        fprintf(stdout,"ft soc Error: %s\n", strError.toStdString().c_str());
        fflush(stdout);
    }
    void slot_stateChanged(QAbstractSocket::SocketState socketState)
    {
        QString strError =
                (socketState == QAbstractSocket::UnconnectedState ?
                     "The socket is not connected." :
                     socketState == QAbstractSocket::HostLookupState ?
                         "The socket is performing a host name lookup." :
                         socketState == QAbstractSocket::ConnectingState ?
                             "The socket has started establishing a connection." :
                             socketState == QAbstractSocket::ConnectedState?
                                 "A connection is established." :
                                 socketState == QAbstractSocket::BoundState?
                                     "The socket is bound to an address and port." :
                                     socketState == QAbstractSocket::ClosingState?
                                         "The socket is about to close (data may still be waiting to be written)." :
                                         socketState == QAbstractSocket::ListeningState?
                                             "The connection was refused.":
                                             "No errors??");

        fprintf(stdout,"Socket %p state %s \n", &socket, strError.toStdString().c_str());
        fflush(stdout);
        //qDebug()<< socketState;
    }

private:

    QFile file;
    QTcpSocket socket;
    bool connected = false;
    qint64 nextBlockSize = 0;
    void error_handler(QString error)
    {
        SendToServer("error: " + error);
    }
    void SendToServer(QString in)
    {
        QByteArray  arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_12);
        out << quint16(0) << in;
        out.device()->seek(0);
        out << quint16(arrBlock.size() - sizeof(quint16));
        //
        if (connected)
            socket.write(arrBlock);
    }
signals:
    void ready();
};

#endif // FILE_TRANSFER_H
