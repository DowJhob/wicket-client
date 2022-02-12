#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::lamp(int a)
{
    switch (a) {
    case 1 : break;
    case 2 : break;
    case 3 : break;
    }
}


void MainWindow::on_PBcert_clicked()
{
    emit send(message(MachineState::undef, command::onBarcode, ui->LEcert->text()));
}


void MainWindow::on_PBcovidC_clicked()
{
    emit send(message(MachineState::undef, command::onBarcode, ui->LEcovidC->text()));
}


void MainWindow::on_PBticket_clicked()
{
    emit send(message(MachineState::undef, command::onBarcode, ui->LEticket->text()));
}


void MainWindow::on_PBticketC_clicked()
{
    emit send(message(MachineState::undef, command::onBarcode, ui->LEticketC->text()));
}


void MainWindow::on_pushButton_clicked()
{
    emit send(message(MachineState::undef, command::onPassed));
}

