#ifndef WICKETLOCKER_H
#define WICKETLOCKER_H

#include <QObject>
#include <QState>
#include <QSignalTransition>
#include <wicketFSM/wicketfsm.h>

class wicketLocker : public QState
{
    Q_OBJECT
public:
    wicketFSM *Armed;
    QState *UnLocked;

    //// Из любого состояния можно перейти в проебана сеть
    QState *SetArmed;
    QState *SetUnLocked;

    wicketLocker(QState *parent):QState(parent)
    {
        SetArmed    = new QState( this );
        Armed       = new wicketFSM(this);
        SetUnLocked = new QState( this );
        UnLocked    = new QState( this );

        //это для безусловного сброса машины  по сигналу с сервера (что бы гасить лампы)
        Armed->addTransition( this, &wicketLocker::from_server_setArmed, Armed );
//=========== Это для принудительной установки состояния если турникет вернет такой сигнал ============================
//======================== например при старте в разблокированном состоянии ===========================================
//============= сохраним для возможности удалить в случае если это ВЫХОДНОЙ считыватель ===============================
        ArmedToUnLockedTransition = Armed->addTransition( this, &wicketLocker::from_crsbrd_unlock, UnLocked );
        UnLockedToArmedTransition = UnLocked->addTransition( this, &wicketLocker::from_crsbrd_armed, Armed );
///================================================================================
        // тут пошел нормальный цикл состояний// через сеттеры
        SetUnLockedToUnLockedTransition =        //сохраним для возможности удалить в случае если это ВЫХОДНОЙ считыватель
                SetUnLocked->addTransition( this, &wicketLocker::from_crsbrd_unlock, UnLocked);    // тут ждем когда разблокируется по сигналу с кросборды

        UnLocked->addTransition(this, &wicketLocker::from_server_setArmed, SetArmed);    // тут подаем кросборде сигнал на взведение турникета
        SetArmedToArmedTransition =        //сохраним для возможности удалить в случае если это ВЫХОДНОЙ считыватель
                SetArmed->addTransition(this, &wicketLocker::from_crsbrd_armed, Armed);            // тут ждем когда турникет будет готов по сигналу с кросборды
        Armed->addTransition(this, &wicketLocker::from_server_setUnLocked, SetUnLocked); // тут подаем кросборде сигнал на разблокирование турникета

        //=================================================================================================

        setInitialState( Armed );
    }
    void set_type_OUT()                          //Переключаем в режим на ВЫХОД
    {

 //https://stackoverflow.com/questions/26331628/reference-to-non-static-member-function-must-be-called
            //==================== отключим кросборду от автомата =================================
            Armed->removeTransition( ArmedToUnLockedTransition );
            UnLocked->removeTransition( UnLockedToArmedTransition );

            SetUnLocked->removeTransition( SetUnLockedToUnLockedTransition);
            SetArmed->removeTransition( SetArmedToArmedTransition );

            SetUnLocked->addTransition( UnLocked ); //делаем их безусловными что бы серверные сигналы не переключать
            SetArmed->addTransition( Armed );

        emit from_crsbrd_armed(); // гарантированно взводим
    }

signals:
    void from_crsbrd_unlock();
    void from_crsbrd_armed();
    void from_server_setUnLocked();
    void from_server_setArmed();

 //   void in_uncond();

private:
    QSignalTransition *ArmedToUnLockedTransition;
    QSignalTransition *UnLockedToArmedTransition;

    QSignalTransition *SetUnLockedToUnLockedTransition;
    QSignalTransition *SetArmedToArmedTransition;
    };

#endif // WICKETLOCKER_H
