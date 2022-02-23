#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H
#include <QtGui>
#include <QVBoxLayout>
#include <QLabel>
//#include <QPlainTextEdit>
//#include <QMovie>
#include <QFont>
#include <QTimer>

#include "../common_types.h"
#include "../command.h"

class showStateWgt: public QWidget
{
    Q_OBJECT
public:

    showStateWgt(QLabel *caption);

public slots:
    void showState(message msg);

    void log(QString str);

private:
    void _showState(QString main_style, QString log_style, QString main_text, QString log_text, QPalette palette, QFont f);

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
