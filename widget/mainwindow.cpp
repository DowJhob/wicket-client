#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSpacerItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), spacer(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::logger(QString in)
{
    QString tmp_cross;
    if( cross )
        tmp_cross = cross_bg_style_true;
    else
        tmp_cross = cross_bg_style_false;
    ui->common_log->append( tmp_cross + QDate::currentDate().toString() + " / " + time_bg_style + QTime::currentTime().toString() + "</span>" + " : " + tmp_cross + in + "</span>");
    cross = !cross;
}

//  на выходе индекс таба с таким именем
int MainWindow::addTab( QString name )
{
    int i = 0;
    for ( i = 0; i < ui->tabWidget->count(); i++ )
        if ( ui->tabWidget->widget( i ) != nullptr )
        {
            if ( name == ui->tabWidget->tabText(i) )
                return i;
        }
        else
            break;
    ui->tabWidget->addTab( new QWidget( ui->tabWidget ), name );
    QGridLayout *g = new QGridLayout(ui->tabWidget);
    g->setMargin(0);
    g->setSpacing(0);
    ui->tabWidget->widget(i)->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tabWidget->widget(i)->setLayout( g );

    QWidget *w = new QWidget(ui->tabWidget);
    QHBoxLayout *h = new QHBoxLayout(w);
    w->setLayout(h);
    h->setMargin(0);
    h->setSpacing(0);

    QPushButton *armedButton = new QPushButton("Armed", ui->tabWidget);
    QPushButton *unlockButton = new QPushButton("Unlock", ui->tabWidget);
    h->addWidget(armedButton);
    h->addWidget(unlockButton);
    g->addWidget( w, 0, 0 );
    g->addItem( new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding), 300, 0);

    connect(armedButton, &QPushButton::clicked, this, &MainWindow::tabArmed);
    connect(unlockButton, &QPushButton::clicked, this, &MainWindow::tabUnlock);

    return i;
}
void MainWindow::tabArmed()
{
    QPushButton *armbutt = qobject_cast<QPushButton*>(sender());
    //    QTabWidget *tbwgt = qobject_cast<QTabWidget*>( armbutt->parent() );
    QObjectList l =  armbutt->parent()->parent()->children();
    foreach ( QObject *www, l )
        if ( www->objectName() == "wicket_widget")
        {
            wicket_widget *ww = qobject_cast<wicket_widget*>( www );
            emit lock_unlock("armed=" + ww->get_addr());

        }
}

void MainWindow::tabUnlock()
{
    QPushButton *unlbutt = qobject_cast<QPushButton*>(sender());
    //    QTabWidget *tbwgt = qobject_cast<QTabWidget*>( armbutt->parent() );
    QObjectList l =  unlbutt->parent()->parent()->children();
    foreach ( QObject *www, l )
        if ( www->objectName() == "wicket_widget")
        {
            wicket_widget *ww = qobject_cast<wicket_widget*>( www );
            emit lock_unlock("unlock=" + ww->get_addr());

        }
}
void MainWindow::readyRead(QString str)
{
//    emit reconnect_reset();
    QStringList lst = str.split("///");
    QString msg_str;
    QString addr;
    if ( lst.count() >=2 )
    {
        addr =  lst.at(0);
        msg_str = lst.at(1);
    }

    QStringList lst2 = msg_str.split(":");
    QString GateGroupCaption{};
    QString GateNumber{};
    QString readerDirectionType{};
    if ( lst2.count() >=4 )
    {
        GateGroupCaption =  lst2.at(0);
        GateNumber = lst2.at(1);
        readerDirectionType = lst2.at(2);
        msg_str = lst2.at(3);
    }
    int num_num = GateNumber.toInt();
    //============================================================================================================
    int num_tab = addTab(GateGroupCaption); ///если такой закладки еще нет - создадим
    QLayout *tabLayot = ui->tabWidget->widget( num_tab )->layout();

    wicket_widget *ww = nullptr;

    QGridLayout *g = reinterpret_cast<QGridLayout*>( tabLayot );
    QLayoutItem *it = g->itemAtPosition( num_num, 0 );
    if ( it == nullptr )
    {
        if ( readerDirectionType == "1" )
            addr = "";
        ww = new wicket_widget(ui->tabWidget->widget( num_tab ), addr, GateNumber);

        //        connect(ww, SIGNAL(lock_unlock(QString)), SLOT(lock_unlock(QString)));
        //        connect(ww, SIGNAL(test_mode(QString)), SLOT(test_mode(QString)));
        g->addWidget( ww, num_num, 0);                      // добавляем виджет
    }
    else {
        ww = reinterpret_cast<wicket_widget*>( it->widget() );
        if ( readerDirectionType == "1" )
            ww->reset_exit_status_timer();
        if ( readerDirectionType == "0" || readerDirectionType == "2" )
        {
            if ( ww->get_addr().isEmpty() )
                ww->set_addr(addr);
            ww->reset_entry_status_timer();
        }
    }

    //============================================================================================================
    QStringList param_list = msg_str.split(";");

    foreach (QString param, param_list)
        parser(param, ww, readerDirectionType);

}
void MainWindow::parser(QString param, wicket_widget *wicket_widget_obj, QString readerDirectionType)
{
    if ( param.indexOf("register") >= 0 )
        logger("register wicket: " + wicket_widget_obj->get_addr());
    if ( param.indexOf("nikiret error") >= 0 )
        logger(" nikiret error: " + wicket_widget_obj->get_addr() );

    int pos = param.indexOf("barcode");
    if ( pos >= 0 )
    {
        QString barcode = param.mid( pos + 8 );
        if ( !barcode.isEmpty() )
        {
            if ( readerDirectionType == "1")
                wicket_widget_obj->set_exit_barcode( barcode );
            if ( readerDirectionType == "0" || readerDirectionType == "2" )
                wicket_widget_obj->set_entry_barcode( barcode );
        }
        wicket_widget_obj->controller_state = state::busy_send_entry_barcode;
        //logger( wicket_widget_obj->get_addr() + " | barcode: " + wicket_widget_obj->get_entry_barcode() );
    }
    if ( param.indexOf("entry-passed") >= 0 )
    {
        wicket_widget_obj->controller_state = state::busy_wait_pass_entry;
        //помигать?
    }
    if ( param.indexOf("exit-passed") >= 0 )
    {
        wicket_widget_obj->controller_state = state::busy_wait_pass_exit;

    }
    if ( param.indexOf("pass-dropped") >= 0 )
        wicket_widget_obj->controller_state = state::ready;

    int temp_pos = param.indexOf("TEMP");
    if ( temp_pos >= 0 )
    {
        if ( readerDirectionType == "1")
            wicket_widget_obj->set_exit_temp( param.mid( temp_pos + 5, 5 ) +(" ℃") );
        if ( readerDirectionType == "0" || readerDirectionType == "2" )
            wicket_widget_obj->set_entry_temp( param.mid( temp_pos + 5, 5 ) +(" ℃") );
    }
    if ( (param.indexOf("ready") >= 0) || (param.indexOf("wicket_status=0") >= 0) )
    {
        if ( readerDirectionType == "1" )
        {
            wicket_widget_obj->set_exit_status_ready(wicket_widget_obj);
        }
        if ( readerDirectionType == "0" || readerDirectionType == "2" )
        {
            wicket_widget_obj->set_entry_status_ready(wicket_widget_obj);
        }

    }
    if ( (param.indexOf("unlock") >= 0) || (param.indexOf("wicket_status=1") >= 0) )
    {

        if ( readerDirectionType == "1" )
        {
            wicket_widget_obj->set_exit_status_unlock(wicket_widget_obj);
        }
        if ( readerDirectionType == "0" || readerDirectionType == "2" )
        {
            wicket_widget_obj->set_entry_status_unlock(wicket_widget_obj);
        }









    }
}

void MainWindow::add_wicket(QTcpSocket* pClientSocket)
{

}

void MainWindow::on_refreshButton_clicked( )
{
    emit send_to_socket("refresh");
}

void MainWindow::on_normalButton_clicked()
{
    emit send_to_socket("normal_mode");
}


void MainWindow::on_unlockAll_pushButton_clicked()
{
    emit send_to_socket("unlock_all");
}

void MainWindow::on_armedAll_pushButton_clicked()
{
    emit send_to_socket("armed_all");
}
