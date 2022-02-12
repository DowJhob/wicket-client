/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QGroupBox *groupBox_7;
    QGridLayout *gridLayout_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QLineEdit *LEcert;
    QPushButton *PBcert;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_3;
    QLineEdit *LEcovidC;
    QPushButton *PBcovidC;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_4;
    QLineEdit *LEticket;
    QPushButton *PBticket;
    QGroupBox *groupBox_6;
    QVBoxLayout *verticalLayout_6;
    QLineEdit *LEticketC;
    QPushButton *PBticketC;
    QPushButton *pushButton;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(684, 647);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        groupBox_7 = new QGroupBox(centralwidget);
        groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));
        gridLayout_2 = new QGridLayout(groupBox_7);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        groupBox = new QGroupBox(groupBox_7);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        LEcert = new QLineEdit(groupBox);
        LEcert->setObjectName(QString::fromUtf8("LEcert"));

        verticalLayout->addWidget(LEcert);

        PBcert = new QPushButton(groupBox);
        PBcert->setObjectName(QString::fromUtf8("PBcert"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(PBcert->sizePolicy().hasHeightForWidth());
        PBcert->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(PBcert);


        gridLayout_2->addWidget(groupBox, 0, 0, 1, 1);

        groupBox_3 = new QGroupBox(groupBox_7);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout_3 = new QVBoxLayout(groupBox_3);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        LEcovidC = new QLineEdit(groupBox_3);
        LEcovidC->setObjectName(QString::fromUtf8("LEcovidC"));

        verticalLayout_3->addWidget(LEcovidC);

        PBcovidC = new QPushButton(groupBox_3);
        PBcovidC->setObjectName(QString::fromUtf8("PBcovidC"));
        sizePolicy.setHeightForWidth(PBcovidC->sizePolicy().hasHeightForWidth());
        PBcovidC->setSizePolicy(sizePolicy);

        verticalLayout_3->addWidget(PBcovidC);


        gridLayout_2->addWidget(groupBox_3, 1, 0, 1, 1);

        groupBox_4 = new QGroupBox(groupBox_7);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        verticalLayout_4 = new QVBoxLayout(groupBox_4);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        LEticket = new QLineEdit(groupBox_4);
        LEticket->setObjectName(QString::fromUtf8("LEticket"));

        verticalLayout_4->addWidget(LEticket);

        PBticket = new QPushButton(groupBox_4);
        PBticket->setObjectName(QString::fromUtf8("PBticket"));
        sizePolicy.setHeightForWidth(PBticket->sizePolicy().hasHeightForWidth());
        PBticket->setSizePolicy(sizePolicy);

        verticalLayout_4->addWidget(PBticket);


        gridLayout_2->addWidget(groupBox_4, 2, 0, 1, 1);

        groupBox_6 = new QGroupBox(groupBox_7);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        verticalLayout_6 = new QVBoxLayout(groupBox_6);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        LEticketC = new QLineEdit(groupBox_6);
        LEticketC->setObjectName(QString::fromUtf8("LEticketC"));

        verticalLayout_6->addWidget(LEticketC);

        PBticketC = new QPushButton(groupBox_6);
        PBticketC->setObjectName(QString::fromUtf8("PBticketC"));
        sizePolicy.setHeightForWidth(PBticketC->sizePolicy().hasHeightForWidth());
        PBticketC->setSizePolicy(sizePolicy);

        verticalLayout_6->addWidget(PBticketC);


        gridLayout_2->addWidget(groupBox_6, 3, 0, 1, 1);


        gridLayout->addWidget(groupBox_7, 0, 0, 1, 1);

        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        gridLayout->addWidget(pushButton, 0, 1, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 684, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        groupBox_7->setTitle(QApplication::translate("MainWindow", "\320\222\321\205\320\276\320\264", nullptr));
        groupBox->setTitle(QString());
        LEcert->setText(QApplication::translate("MainWindow", "https://www.gosuslugi.ru/api/covid-cert/v2/cert/status/52593579-7a51-43c5-bfaf-c9bde5d5f646?lang=ru", nullptr));
        PBcert->setText(QApplication::translate("MainWindow", "\320\241\320\265\321\200\321\202\320\270\321\204\320\270\320\272\320\260\321\202", nullptr));
        groupBox_3->setTitle(QString());
        LEcovidC->setText(QApplication::translate("MainWindow", "covidControllerPrefix:288B3DE5-2734-440D-9DDA-9A4D9025F179", nullptr));
        PBcovidC->setText(QApplication::translate("MainWindow", "\320\232\320\276\320\262\320\270\320\264\320\232\320\276\320\275\321\202\321\200\320\276\320\273\320\265\321\200", nullptr));
        groupBox_4->setTitle(QString());
        LEticket->setText(QApplication::translate("MainWindow", "superticket", nullptr));
        PBticket->setText(QApplication::translate("MainWindow", "\320\221\320\270\320\273\320\265\321\202", nullptr));
        groupBox_6->setTitle(QString());
        PBticketC->setText(QApplication::translate("MainWindow", "\320\221\320\270\320\273\320\265\321\202\320\275\321\213\320\271 \320\272\320\276\320\275\321\202\321\200\320\276\320\273\320\265\321\200", nullptr));
        pushButton->setText(QApplication::translate("MainWindow", "PushButton", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
