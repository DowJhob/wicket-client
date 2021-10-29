#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H
#include <QtGui>
#include <QVBoxLayout>
#include <QLabel>
//#include <QPlainTextEdit>
#include <QMovie>
#include <QFont>
#include <QTimer>

#include <common_types.h>

class picture2: public QWidget
{
    Q_OBJECT
public:

    picture2(QString IPaddr)
    {
        IP.setText( IPaddr );
    }

public slots:

    void start()
    {
        access_pixmap.load(":images/access-4.png");
        place_ticket_pixmap.load(":images/place_ticket-4.png");
        denied_pixmap.load(":images/denied-2.png");
        oncheck_pixmap.load(":images/oncheck.png");
        service_pixmap.load(":images/service.jpg");

        setFixedWidth( 480 );
        setFixedHeight( 640 );
        setLayout( &background_layout );
        showMaximized();

        access_palette.setBrush(QPalette::Background, access_pixmap);
        place_palette.setBrush(QPalette::Background, place_ticket_pixmap);
        denied_palette.setBrush(QPalette::Background, denied_pixmap);
        service_palette.setBrush(QPalette::Background, service_pixmap);
        oncheck_palette.setBrush(QPalette::Background, oncheck_pixmap);

        IP.setFont( _f10 );
        info_log.setWordWrap(true);
        info_log.setFont( _f15 );
        //info_log.setSizePolicy(QSizePolicy::Minimum,
        //                       QSizePolicy::Minimum) ;
        //main_message.setSizePolicy(QSizePolicy::Minimum,
        //                               QSizePolicy::Minimum) ;

        main_message.setFont( QFont("Times New Roman", 19, QFont :: Bold ) );
        main_message.setAlignment( Qt::AlignCenter | Qt::AlignBottom );

        IP.setText("IP");
        info_log.setText("Слава робатам, убить всех человекав!\n");
        main_message.setText("main_message");


        //background_layout.setSizeConstraint(QLayout::SetMinAndMaxSize);
        background_layout.addWidget(&IP, 0, Qt::AlignLeft | Qt::AlignTop );
        background_layout.addWidget(&info_log, 5, Qt::AlignVCenter | Qt::AlignLeft );
        background_layout.addWidget(&main_message, 5, Qt::AlignHCenter | Qt::AlignBottom );

        setPicture(pict_service, "");

    }
    void setPicture(int pict_type, QString ticket_status_description)
    {
        info_log.clear();
        switch (pict_type) {
        case pict_service : set_picture_com(black_style, black_style, "Пожалуйста подождите,\nидет настройка оборудования", ticket_status_description, service_palette, _f10);
            break;
        case pict_ready   : set_picture_com(black_style, black_style, "Поднесите билет\nPlease place the ticket", ticket_status_description, place_palette, _f10);
            break;
        case pict_onCheck : set_picture_com(black_style, black_style, "Подождите, проверяем\nваш билет\nPlease wait, check\n your ticket", ticket_status_description, oncheck_palette, _f10);
            break;
        case pict_access  : set_picture_com(black_style, black_style, "Пожалуйста проходите\nPlease come in", ticket_status_description, access_palette, _f10);
            break;
        case pict_timeout : set_picture_com(black_style, black_style, "Пожалуйста подождите,\nохрана досматривает\nвпереди идущего", ticket_status_description, service_palette, _f10);
            break;
        case pict_denied  : set_picture_com(yellow_style, yellow_style, "Доступ запрещен\nAccess denied", ticket_status_description, denied_palette, _f15);
            break;

        }
    }

    void log(QString str)
    {
        fprintf(stdout, "%s", str.toStdString().c_str() );
        fflush(stdout);
        main_message.setStyleSheet("QLabel {  color : black; }");
        info_log.setStyleSheet("QLabel {  color : black; }");
        info_log.setText( info_log.text() + str );
    }

private:
    void set_picture_com(QString main_style, QString log_style, QString main_text, QString log_text, QPalette palette, QFont f)
    {
        info_log.setFont( f );
        info_log.setStyleSheet(log_style);
        info_log.setText( log_text );

        main_message.setStyleSheet(main_style);
        main_message.setText(main_text);
        setPalette(palette);
    }

    QString black_style = "QLabel{ color:black;}";
    QString yellow_style = "QLabel {  color : yellow; }";
    QFont _f10 = QFont("Times New Roman", 10, QFont :: Bold );
    QFont _f15 = QFont("Times New Roman", 15, QFont :: Bold );


    QVBoxLayout background_layout;

    QLabel IP;
    QLabel info_log;
    QLabel main_message;
    //QLabel IP;

    QPalette access_palette;
    QPalette place_palette;
    QPalette denied_palette;
    QPalette service_palette;
    QPalette oncheck_palette;

    QPixmap place_ticket_pixmap;
    QPixmap oncheck_pixmap;
    QPixmap access_pixmap;
    QPixmap denied_pixmap;
    QPixmap service_pixmap;

};
#endif // MAIN_WIDGET_H
