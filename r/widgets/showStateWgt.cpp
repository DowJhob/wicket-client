#include "showStateWgt.h"

showStateWgt::showStateWgt()
{
    //IP.setText( IPaddr );
}

void showStateWgt::start()
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

    showState(black_style, black_style,   "Пожалуйста подождите,\nидет настройка оборудования",
                                                    "", service_palette, _f10);

}

void showStateWgt::showStatus(message msg)
{
    info_log.clear();
    command cmd = msg.cmd;
    QString desc = msg.body.toString();
    switch (cmd) {
    case command::showServiceStatus : showState(black_style, black_style, desc, "", service_palette, _f10);
        break;
    case command::showReadyStatus   : showState(black_style, black_style, desc, "", place_palette, _f10); qDebug() << " showReadyStatus" << desc;
        break;
    case command::showOpenStatus    : showState(black_style, black_style, desc, "", access_palette, _f10); qDebug() << " showOpenStatus" << desc;
        break;
        //==========================================
    case command::showPlaceStatus   : showState(black_style, black_style, desc, "", place_palette, _f10); qDebug() << " showPlaceStatus" << desc;
        break;
    case command::showCheckStatus   : showState(black_style, black_style, desc, "", oncheck_palette, _f10); qDebug() << " showCheckStatus" << desc;
        break;
    case command::showFailStatus    : showState(black_style, black_style, desc, "", denied_palette, _f10); qDebug() << " showFailStatus" << desc;
        break;
    default:break;

    }
}

void showStateWgt::log(QString str)
{
    fprintf(stdout, "%s", str.toStdString().c_str() );
    fflush(stdout);
    main_message.setStyleSheet("QLabel {  color : black; }");
    info_log.setStyleSheet("QLabel {  color : black; }");
    info_log.setText( info_log.text() + str );
}

void showStateWgt::showState(QString main_style, QString log_style, QString main_text, QString log_text, QPalette palette, QFont f)
{
    info_log.setFont( f );
    info_log.setStyleSheet(log_style);
    info_log.setText( log_text );

    main_message.setStyleSheet(main_style);
    main_message.setText(main_text);
    setPalette(palette);
}
