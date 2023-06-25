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
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QFrame *line;
    QPushButton *pushButton_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *unlockAll_pushButton;
    QPushButton *refreshButton;
    QPushButton *normalButton;
    QPushButton *armedAll_pushButton;
    QTextEdit *common_log;
    QGroupBox *ctrlbtnGroupBox;
    QGridLayout *gridLayout;
    QTabWidget *tabWidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1000, 400);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMinimumSize(QSize(1000, 400));
        MainWindow->setBaseSize(QSize(1900, 1000));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
        centralwidget->setSizePolicy(sizePolicy1);
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy2);
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        line = new QFrame(groupBox);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout_2->addWidget(line, 7, 1, 1, 2);

        pushButton_2 = new QPushButton(groupBox);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(pushButton_2->sizePolicy().hasHeightForWidth());
        pushButton_2->setSizePolicy(sizePolicy3);
        pushButton_2->setAutoDefault(false);
        pushButton_2->setFlat(false);

        gridLayout_2->addWidget(pushButton_2, 4, 1, 1, 2);

        horizontalSpacer = new QSpacerItem(40, 5, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 8, 1, 1, 2);

        unlockAll_pushButton = new QPushButton(groupBox);
        unlockAll_pushButton->setObjectName(QString::fromUtf8("unlockAll_pushButton"));
        sizePolicy.setHeightForWidth(unlockAll_pushButton->sizePolicy().hasHeightForWidth());
        unlockAll_pushButton->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(unlockAll_pushButton, 2, 1, 1, 1);

        refreshButton = new QPushButton(groupBox);
        refreshButton->setObjectName(QString::fromUtf8("refreshButton"));
        sizePolicy3.setHeightForWidth(refreshButton->sizePolicy().hasHeightForWidth());
        refreshButton->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(refreshButton, 5, 1, 1, 2);

        normalButton = new QPushButton(groupBox);
        normalButton->setObjectName(QString::fromUtf8("normalButton"));
        sizePolicy3.setHeightForWidth(normalButton->sizePolicy().hasHeightForWidth());
        normalButton->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(normalButton, 6, 1, 1, 2);

        armedAll_pushButton = new QPushButton(groupBox);
        armedAll_pushButton->setObjectName(QString::fromUtf8("armedAll_pushButton"));
        sizePolicy.setHeightForWidth(armedAll_pushButton->sizePolicy().hasHeightForWidth());
        armedAll_pushButton->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(armedAll_pushButton, 2, 2, 1, 1);

        common_log = new QTextEdit(groupBox);
        common_log->setObjectName(QString::fromUtf8("common_log"));
        sizePolicy.setHeightForWidth(common_log->sizePolicy().hasHeightForWidth());
        common_log->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(common_log, 0, 1, 1, 2);


        horizontalLayout->addWidget(groupBox);

        ctrlbtnGroupBox = new QGroupBox(centralwidget);
        ctrlbtnGroupBox->setObjectName(QString::fromUtf8("ctrlbtnGroupBox"));
        sizePolicy1.setHeightForWidth(ctrlbtnGroupBox->sizePolicy().hasHeightForWidth());
        ctrlbtnGroupBox->setSizePolicy(sizePolicy1);
        ctrlbtnGroupBox->setMinimumSize(QSize(0, 0));
        gridLayout = new QGridLayout(ctrlbtnGroupBox);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        tabWidget = new QTabWidget(ctrlbtnGroupBox);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy4);
        tabWidget->setMinimumSize(QSize(250, 0));
        tabWidget->setMovable(true);

        gridLayout->addWidget(tabWidget, 0, 0, 1, 1);


        horizontalLayout->addWidget(ctrlbtnGroupBox);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1000, 21));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        pushButton_2->setDefault(false);
        refreshButton->setDefault(false);
        tabWidget->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "wicketWidget", nullptr));
        groupBox->setTitle(QApplication::translate("MainWindow", "Server log", nullptr));
        pushButton_2->setText(QApplication::translate("MainWindow", "Iron mode = off", nullptr));
        unlockAll_pushButton->setText(QApplication::translate("MainWindow", "UNLOCK", nullptr));
        refreshButton->setText(QApplication::translate("MainWindow", "Refresh", nullptr));
        normalButton->setText(QApplication::translate("MainWindow", "Normal mode (for test and register gate mode )", nullptr));
        armedAll_pushButton->setText(QApplication::translate("MainWindow", "ARMED", nullptr));
        ctrlbtnGroupBox->setTitle(QApplication::translate("MainWindow", "Control", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
