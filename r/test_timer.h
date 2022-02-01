#ifndef TEST_TIMER_H
#define TEST_TIMER_H

#include <QObject>
#include <QTimer>
#include "command.h"

class test_timer : public QObject
{
    Q_OBJECT
public:
    explicit test_timer(QObject *parent = nullptr) : QObject(parent)
    {
        connect(&t, &QTimer::timeout, this, &test_timer::r);
                t.start(1000);
    }
    private slots:
    void r()
    {
        emit test(message());
    }
private:
    QTimer t;
signals:
    void test(message);

};

#endif // TEST_TIMER_H
