#ifndef MAINSTACKEDWGT_H
#define MAINSTACKEDWGT_H

#include <QStackedWidget>
#include <QObject>

#include <command.h>
#include "showStateWgt.h"
#include "showCertInfo.h"

class mainStackedWgt : public QStackedWidget
{
    Q_OBJECT
public:
    mainStackedWgt();

public slots:
    void showState(message msg);

private:
    showStateWgt stWgt;
    showCertInfo crtWgt;

signals:
        void log(QString);
};

#endif // MAINSTACKEDWGT_H
