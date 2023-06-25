#ifndef WICKET_WIDGET_H
#define WICKET_WIDGET_H

#include <QObject>
#include <QWidget>
#include <QTcpSocket>
#include <QTime>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QGridLayout>
#include <QPushButton>
#include <QTimer>

#include <common_types.h>

class wicket_widget : public QWidget {
    Q_OBJECT
public:
    int reconnecting_interval = 3000;
    int wicket_status = state::undefined ;
    int controller_state = state::undefined ;

    wicket_widget(QWidget* pwgt = nullptr, QString addr = "wicket addr", QString num = "") : QWidget(pwgt)
    {
        setStyleSheet(QString( "border-radius: 5px; border-style: solid; border-width: 3px; border-color: black; "));
        setParent(pwgt);
        setObjectName("wicket_widget");
        create();
        num_label->setText(num);
        addr_label->setText(addr);

        num_label->setFixedWidth(30);num_label->setFixedHeight(20);
        addr_label->setFixedWidth(80);addr_label->setFixedHeight(20);
        //        lock_unlock_btn->setFixedWidth(50);
        entry_barcode->setFixedWidth(200);entry_barcode->setFixedHeight(20);
        entry_status_label->setFixedWidth(50);entry_status_label->setFixedHeight(20);
        exit_status_label->setFixedWidth(50);exit_status_label->setFixedHeight(20);
        //        exit_state_label->setVisible( false );
        exit_status_label->setEnabled( false );
        connect( &entry_status_timer, &QTimer::timeout, this, &wicket_widget::entry_reconnect);
        connect( &exit_status_timer, &QTimer::timeout, this, &wicket_widget::exit_reconnect);
        connect( &barcode_border_cleaner, &QTimer::timeout, this, &wicket_widget::barcode_border_cleaner_slot);

        entry_status_timer.setSingleShot(true);
        exit_status_timer.setSingleShot(true);
        barcode_border_cleaner.setSingleShot(true);
        entry_status_timer.setInterval(reconnecting_interval);
        exit_status_timer.setInterval(reconnecting_interval);
        barcode_border_cleaner.setInterval(1000);
    }
    ~wicket_widget()
    {

    }
    void set_entry_barcode(QString str)
    {
        entry_barcode->setStyleSheet( entry_barcode_style );
        //barcode_border_cleaner.start();
        entry_barcode->setText(str);
    }
    void set_exit_barcode(QString str)
    {
        entry_barcode->setStyleSheet(QString( exit_barcode_style ));
        //barcode_border_cleaner.start();
        entry_barcode->setText(str);
    }
    QString get_addr()
    {
        return addr_label->text();
    }
    void set_addr(QString addr)
    {
        addr_label->setText(addr);
    }
    void set_entry_temp(QString str)
    {
        entry_status_label->setText(str);
    }
    void set_exit_temp(QString str)
    {
        exit_status_label->setText(str);
    }
    void set_entry_status_ready(wicket_widget *wgt)
    {
        entry_back = "background-color: #aaeeaa;";
        wgt->entry_status_label->setStyleSheet(entry_back + entry_border);
    }
    void set_entry_status_unlock(wicket_widget *wgt)
    {
        entry_back = "background-color: orange;";
        wgt->entry_status_label->setStyleSheet(entry_back + entry_border);
    }
    void set_exit_status_ready(wicket_widget *wgt)
    {
        exit_back = "background-color: #aaeeaa;";
        wgt->exit_status_label->setStyleSheet(exit_back + entry_border);
    }
    void set_exit_status_unlock(wicket_widget *wgt)
    {
        exit_back = "background-color: orange;";
        wgt->exit_status_label->setStyleSheet(exit_back + entry_border);
    }
    QString get_entry_barcode( )
    {
        return entry_barcode->text();
    }

public slots:
    void reset_entry_status_timer()
    {
        entry_border = "border-width: 3px; border-color: #007000;";
        entry_status_label->setStyleSheet(entry_back + entry_border);
        entry_status_label->setEnabled( true );
        entry_status_timer.start();
    }
    void reset_exit_status_timer()
    {
        exit_border = "border-width: 3px; border-color: #007000;";
        exit_status_label->setStyleSheet(exit_back + exit_border);
        exit_status_label->setEnabled( true );
        exit_status_timer.start();
    }

private:
    QString entry_border = "";
    QString exit_border = "";
QString entry_back = "";
QString exit_back = "";

    QTimer entry_status_timer;
    QTimer exit_status_timer;
QTimer barcode_border_cleaner;

    QLabel *num_label = new QLabel("", this);
    QLabel *entry_status_label = new QLabel("", this);
    QLabel *exit_status_label = new QLabel("", this);
    QLabel *entry_barcode = new QLabel("", this);
    QLabel *addr_label = new QLabel("", this);
    QHBoxLayout btn_layout;
    virtual void create()
    {
        setLayout(&btn_layout);
        btn_layout.setSpacing(0);
        btn_layout.setContentsMargins(-5, 2, -5, 2);
        num_label->setStyleSheet(QString( "border-width: 1px;"));
        num_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        btn_layout.addWidget(num_label);

        //lock_unlock_btn->setDisabled(true);
        //        btn_layout.addWidget(lock_unlock_btn);
        btn_layout.addWidget(entry_status_label);
        btn_layout.addWidget(exit_status_label);
        exit_status_label->setStyleSheet(QString( "border-width: 0px;"));
        entry_status_label->setStyleSheet(QString("border-width: 0px;"));
        entry_status_label->setDisabled(true);
        exit_status_label->setDisabled(true);
        //        connect(lock_unlock_btn, SIGNAL(clicked()), SLOT(lock_unlock_slot()));

        //        btn_flash_yellow_timer->setInterval(btn_flash_timer_interval);
        //btn_flash_orange_timer->setInterval(btn_flash_timer_interval);

        //        connect(btn_flash_yellow_timer, SIGNAL(timeout()), SLOT(flasher_timer_slot()));
        //        btn_flash_yellow_timer->setProperty("toggle", 1);

        entry_barcode->setStyleSheet(QString( "border-width: 1px;"));
        entry_barcode->setTextInteractionFlags(Qt::TextSelectableByMouse);
        btn_layout.addWidget(entry_barcode);

        addr_label->setStyleSheet(QString( "border-width: 1px;"));
        addr_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        btn_layout.addWidget(addr_label);

    }
    QString entry_barcode_style = "border-width: 3px; border-color: #005500;";
    QString exit_barcode_style = "border-width: 3px; border-color: orange;";
    QString style_sheet_on;
    QString style_sheet_off;

private slots:
    void entry_reconnect()
    {
        if ( exit_status_label->isEnabled() )
        {
            entry_status_label->setEnabled( false );
            entry_status_label->setStyleSheet(QString( ""));
        }
        else {
            this->deleteLater();
        }

    }
    void exit_reconnect()
    {
        if ( entry_status_label->isEnabled() )
        {
            exit_status_label->setEnabled( false );
            exit_status_label->setStyleSheet(QString( ""));
        }
        else {
            this->deleteLater();
        }
    }
    void barcode_border_cleaner_slot()
    {
        entry_barcode->setStyleSheet(QString( "border-width: 1px; border-color: black; "));
    }
    void lock_unlock_slot()
    {
        QPushButton* pushButton = static_cast<QPushButton*>(sender());  // источник кнопа
        wicket_widget *obj = reinterpret_cast<wicket_widget*>( pushButton->parent() );
//        set_ready_flash();
        //obj->btn_flash_yellow_timer->start();
        emit lock_unlock("lock_unlock_addr=" + obj->get_addr());                                                  //
    }
    void flasher_timer_slot()
    {
        QTimer* btn_flash_timer = static_cast<QTimer*>(sender());
        wicket_widget *obj = reinterpret_cast<wicket_widget*>( btn_flash_timer->parent() );
        if (btn_flash_timer->property("toggle") == 1)
        {
            //            obj->flashing_obj->setStyleSheet(style_sheet_on);
            btn_flash_timer->setProperty("toggle", 0);
        }
        else
        {
            //            obj->flashing_obj->setStyleSheet(style_sheet_off);
            btn_flash_timer->setProperty("toggle", 1);
        }
    }
signals:
    void lock_unlock(QString);
    void test_mode(QString);
};

//Q_DECLARE_METATYPE(tt_widget);

#endif // TURNSTILE_WIDGET_H
