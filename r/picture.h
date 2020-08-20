#ifndef PICTURE_H
#define PICTURE_H
#include <QtGui>
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QMovie>
#include <QFont>
#include <QTimer>

class picture: public QObject
{
    Q_OBJECT
public:
    QLabel picture_label;

    picture()
    {

        picture_label.setWordWrap(true);
        picture_label.showMaximized();
        set_picture_service();
    }
    void set_local_octet_to_screen(QString ff)
    {
        picture_label.setAlignment( Qt::AlignBottom | Qt::AlignRight );
        picture_label.setText( ff );
    }
public slots:

    void set_picture_wait()
    {
        //picture_label.setPixmap(wait_pixmap);
    }
    void set_picture_ready()
    {
        info_message("Поднесите билет\n"
                     "Please place the ticket");
        //picture_label.setPixmap(place_ticket_pixmap);
        picture_label.setStyleSheet("background-image: url("":images/place_ticket.jpg"");");
        //picture_label.setMovie(&mv);
        //mv.start();
    }
    void set_picture_acces()
    {
        info_message("Пожалуйста проходите\n"
                     "Please come in");
        //picture_label.setPixmap(access_pixmap);
        picture_label.setStyleSheet("background-image: url("":images/access.jpg"");");
    }
    void set_picture_denied( QString ticket_status_description )
    {
        //picture_label.setPixmap(denied_pixmap);
        picture_label.setStyleSheet("background-image: url("":images/denied.jpg"");");
    }
    void set_picture_help()
    {
        //picture_label.setPixmap(help_pixmap);
    }
    void set_picture_service()
    {
        picture_label.setStyleSheet("background-image: url("":images/service.jpg"");");
        info_message("Пожалуйста подождите,\n"
                     "идет настройка оборудования");
    }
    void putString(QString in)
    {
        QString str = picture_label.text();
        str.remove( 0, str.indexOf("\n") + 1);
        picture_label.setText( str + in  + "\n");
    }
    void log(QString str)
    {
        picture_label.setAlignment( Qt::AlignBottom | Qt::AlignLeft);
        picture_label.setFont( QFont("Times New Roman", 10, QFont :: Bold ) );
        picture_label.setText( picture_label.text() + str );
    }
    void info_message(QString str)
    {
        picture_label.setAlignment (  Qt::AlignHCenter|Qt::AlignTop);
        //
        //picture_label.setStyleSheet( QString("font-size: 30px;") );
        picture_label.setFont( QFont("Times New Roman", 19, QFont :: Bold ) );
        //picture_label.setText( "<font size=6><b>" + str + "</font></b>" );
        picture_label.setText( str );
    }
private:
    QPixmap wait_pixmap;
    QPixmap place_ticket_pixmap;
    QPixmap access_pixmap;
    QPixmap denied_pixmap;
    QPixmap help_pixmap;
    QPixmap service_pixmap;
    QPixmap e_pixmap;
    QPixmap test;



};





#endif // PICTURE_H
