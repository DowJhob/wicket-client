#ifndef WICKET_OBJECT_H
#define WICKET_OBJECT_H
#include <common_types.h>
#include <QObject>
#include <QTimer>
#include <QTcpSocket>

class wicket_reader : public QObject{
    Q_OBJECT
public:
    QString entry_barcode;
    QString exit_barcode;
    QTimer connecting_state_timer;
    int wicket_status = state::undefined ;
    int controller_state = state::undefined ;
    QTcpSocket *socket;
    int connecting_state_interval = 5000;
    quint16      m_nNextBlockSize = 0;
    quint16      ft_nNextBlockSize = 0;
    QString crss_brd_temp;
QString str="";

    wicket_reader(QTcpSocket* pClientSocket = nullptr, QObject* pwgt = nullptr) : QObject(pwgt)
    {
        socket = pClientSocket;

        connect(&connecting_state_timer, SIGNAL(timeout()), this, SLOT(connecting_timer_slot()));
        connecting_state_timer.setInterval(connecting_state_interval);
        connecting_state_timer.setSingleShot(true);

        QVariant qVarian_pointer_to_wicket_object = QVariant::fromValue(this);
        connecting_state_timer.setProperty("wicket_object_addr", qVarian_pointer_to_wicket_object );
        connecting_state_timer.start();

        pClientSocket->setProperty("wicket_object_addr", qVarian_pointer_to_wicket_object );

    }
    ~wicket_reader()
    {
        socket->abort();
        socket->deleteLater();
    }
private slots:
    void connecting_timer_slot()
    {
        QTimer* connecting_state_timer = static_cast<QTimer*>(sender());
        wicket_reader *obj = qvariant_cast<wicket_reader*>( connecting_state_timer->property("wicket_object_addr") );
        obj->deleteLater();
    }
};

#endif // WICKET_OBJECT_H
