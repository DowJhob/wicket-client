#ifndef MAINSTACKEDWGT_H
#define MAINSTACKEDWGT_H

#include <QStackedWidget>
#include <QObject>

#include "../command.h"
#include "showStateWgt.h"
#include "showCertInfo.h"

class mainStackedWgt : public QStackedWidget
{
    Q_OBJECT
public:
    mainStackedWgt();

    void setCaption(QString caption)
    {
        this->caption.setText(caption);
    }

public slots:
    void showState(message msg);

private:
    QLabel caption;
    QLabel version;
    showStateWgt stWgt{&caption};
    showCertInfo crtWgt;

signals:
    void log(QString);

};

#endif // MAINSTACKEDWGT_H
