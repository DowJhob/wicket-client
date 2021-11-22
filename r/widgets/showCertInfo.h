#ifndef SHOWCERTINFO_H
#define SHOWCERTINFO_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

class showCertInfo : public QWidget
{
    Q_OBJECT
public:
    showCertInfo();

private:
    QVBoxLayout background_layout;
    QPalette palette;
    QPixmap pixmap;

    QLabel certNumber;
    QLabel Expired;
    QLabel FIO;
    QLabel Passport;

};

#endif // SHOWCERTINFO_H
