#include "showStateWgt.h"

showStateWgt::showStateWgt()
{
    access_pixmap.load(":images/access-4.png");
    place_ticket_pixmap.load(":images/place_ticket-4.png");
    denied_pixmap.load(":images/denied-2.png");
    oncheck_pixmap.load(":images/oncheck.png");
    service_pixmap.load(":images/service.jpg");

    setAutoFillBackground(true);
    setFixedWidth( 480 );
    setFixedHeight( 640 );
    setLayout( &background_layout );
    showMaximized();

    access_palette.setBrush(QPalette::Background, access_pixmap);
    place_palette.setBrush(QPalette::Background, place_ticket_pixmap);
    denied_palette.setBrush(QPalette::Background, denied_pixmap);
    service_palette.setBrush(QPalette::Background, service_pixmap);
    oncheck_palette.setBrush(QPalette::Background, oncheck_pixmap);

    caption.setFont( _f10 );
    info_log.setWordWrap(true);
    info_log.setFont( _f15 );

    main_message.setWordWrap(true);
    main_message.setFont( QFont("Times New Roman", 18, QFont :: Bold ) );
    main_message.setAlignment( Qt::AlignCenter | Qt::AlignBottom );

    main_message.setText("Пожалуйста подождите, идет настройка оборудования");
    setPalette(service_palette);



    info_layout.addWidget(&caption, 0, Qt::AlignLeft | Qt::AlignTop );
    info_layout.addWidget(&version, 0, Qt::AlignRight | Qt::AlignTop );


    background_layout.addLayout(&info_layout);



    background_layout.addWidget(&info_log, 5, Qt::AlignVCenter | Qt::AlignLeft );
    background_layout.addWidget(&main_message, 5, Qt::AlignHCenter | Qt::AlignBottom );
}

void showStateWgt::showState(message msg)
{
    info_log.clear();
    command cmd = msg.cmd;
    QString desc = msg.body.toString();
    switch (cmd) {
    case command::showServiceStatus : _showState(black_style, black_style, desc, "", service_palette, _f10);
        break;
    case command::showReadyStatus   : _showState(black_style, black_style, desc, "", place_palette, _f10); qDebug() << " showReadyStatus" << desc;
        break;
    case command::showOpenStatus    : _showState(black_style, black_style, desc, "", access_palette, _f10); qDebug() << " showOpenStatus" << desc;
        break;
        //==========================================
    case command::showPlaceStatus   : _showState(black_style, black_style, desc, "", place_palette, _f10); qDebug() << " showPlaceStatus" << desc;
        break;
    case command::showCheckStatus   : _showState(black_style, black_style, desc, "", oncheck_palette, _f10); qDebug() << " showCheckStatus" << desc;
        break;
    case command::showFailStatus    : _showState(black_style, black_style, desc, "", denied_palette, _f10); qDebug() << " showFailStatus" << desc;
        break;
    default:break;

    }
}

void showStateWgt::log(QString str)
{
    fprintf(stdout, "%s", str.toStdString().c_str() );
    fflush(stdout);
    //main_message.setStyleSheet("QLabel {  color : black; }");
    info_log.setStyleSheet("QLabel {  color : black; }");
    info_log.setText( info_log.text() + str );
}

void showStateWgt::_showState(QString main_style, QString log_style, QString main_text, QString log_text, QPalette palette, QFont f)
{
    info_log.setFont(f);
    info_log.setStyleSheet(log_style);
    info_log.setText(log_text);

    main_message.setStyleSheet(main_style);
    main_message.setText(main_text);
    setPalette(palette);
}
