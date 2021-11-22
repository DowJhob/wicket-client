#ifndef MAINSTACKEDWGT_H
#define MAINSTACKEDWGT_H

#include <QStackedWidget>
#include <QObject>
#include "showStateWgt.h"
#include "showCertInfo.h"

class mainStackedWgt : public QStackedWidget
{
    Q_OBJECT
public:
    mainStackedWgt();

private:
    showStateWgt stWgt;
    showCertInfo crtWgt;


};

#endif // MAINSTACKEDWGT_H
