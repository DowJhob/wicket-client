#include "mainStackedWgt.h"

mainStackedWgt::mainStackedWgt()
{
    setAutoFillBackground(true);
    connect(this, &mainStackedWgt::log, &stWgt, &showStateWgt::log);
    addWidget(&stWgt);
    addWidget(&crtWgt);

    showMaximized();
}

void mainStackedWgt::showState(message msg)
{
    command cmd = msg.cmd;
    if(cmd == command::showInfoStatus)
    {
        crtWgt.setInfo(msg);
        setCurrentIndex(1);
    }
    else
    {
        stWgt.showState(msg);
        setCurrentIndex(0);
    }
}
