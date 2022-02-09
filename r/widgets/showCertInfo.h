#ifndef SHOWCERTINFO_H
#define SHOWCERTINFO_H

#include <QObject>
#include <QDebug>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <command.h>

class showCertInfo : public QWidget
{
    Q_OBJECT
public:
    showCertInfo();

public slots:
    void setInfo(message msg);

private:
    QVBoxLayout background_layout;
    QPalette palette;
    QPixmap pixmap;

    // ================
    QVBoxLayout certInfo_layout;
    QLabel certNumber;
    QLabel Expired;

    // ================
    QVBoxLayout pers_layout;
    QLabel FIO;
    QLabel birthday;
    QLabel Passport;

};

#endif // SHOWCERTINFO_H
