#include "showCertInfo.h"

showCertInfo::showCertInfo()
{
    setAutoFillBackground(true);
    setFixedWidth( 480 );
    setFixedHeight( 640 );
    setLayout( &background_layout );
    pixmap.load(":images/access-4.png");
    palette.setBrush(QPalette::Background, pixmap);
    setPalette(palette);

    certInfo_layout.addWidget(&certNumber);
    certInfo_layout.addWidget(&Expired);

    FIO.setFont(QFont("Times New Roman", 15, QFont::Bold));
    birthday.setFont(QFont("Times New Roman", 15, QFont::Bold));
    Passport.setFont(QFont("Times New Roman", 15, QFont::Bold));

    pers_layout.addWidget(&FIO);
    pers_layout.addWidget(&birthday);
    pers_layout.addWidget(&Passport);

    background_layout.addLayout(&certInfo_layout);
    background_layout.addLayout(&pers_layout);
    setLayout(&background_layout);

    showMaximized();
}

void showCertInfo::setInfo(message msg)
{
    //QJsonDocument doc = QJsonDocument::fromJson(msg.body.toByteArray());
    //QJsonArray info = doc["attrs"].toArray();
    QVariantList v = msg.body.toList();
    qDebug() << " showInfoStatus" << v;
    for (auto o : v)
    {
        auto oo = o.toMap();

        //auto oo = o.toObject();
        QString s = oo["title"].toString() + " : " + oo["value"].toString();
        if(oo["type"] == "fio")
            FIO.setText(s);
        else if(oo["type"] == "birthDate")
            birthday.setText(s);
        else if(oo["type"] == "passport")
            Passport.setText(s);
        else
            ;
    }
}
