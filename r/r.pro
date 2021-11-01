#QT -= gui
QT += core network
QT += gui
QT += widgets
QT += serialport
#QT += webkitwidgets
#QT += webengine

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
#        PN532-PN532_HSU/PN532/PN532.cpp \
#        PN532-PN532_HSU/PN532/emulatetag.cpp \
#        PN532-PN532_HSU/PN532/llcp.cpp \
#        PN532-PN532_HSU/PN532/mac_link.cpp \
#        PN532-PN532_HSU/PN532/snep.cpp \
#        PN532-PN532_HSU/PN532/PN532_HSU.cpp \
        barcode_reader/libusb-wrapper.cpp \
        controller_2.cpp \
        main-widget.cpp \
        main.cpp \
        network.cpp \
        nikiret.cpp \
        wicketFSM/covid-cert-cheker.cpp \
        wicketFSM/main-fsm.cpp \
        wicketFSM/wicketlocker.cpp \
        wicketFSM/wicketfsm.cpp

INSTALLS += target
target.path = target.path = /usr
target.files = rd

HEADERS += common_types.h \
#    NFC.h \
#    PN532-PN532_HSU/PN532/PN532.h \
#    PN532-PN532_HSU/PN532/PN532Interface.h \
#    PN532-PN532_HSU/PN532/PN532_debug.h \
#    PN532-PN532_HSU/PN532/emulatetag.h \
#    PN532-PN532_HSU/PN532/llcp.h \
#    PN532-PN532_HSU/PN532/mac_link.h \
#    PN532-PN532_HSU/PN532/snep.h \
#    PN532-PN532_HSU/PN532/PN532_HSU.h \
#    NFC_copy.h \
#controller.h \
#    barcode_qt.h \
    barcode_reader/barcode_msg.h \
    barcode_reader/libusb-wrapper.h \
    command.h \
    controller_2.h \
#    htmlwidget.h \
    libs/libusb/include/libusb.h \
    main-widget.h \
    network.h \
    nikiret.h \
    test_timer.h \
#    updater.h \
    wicketFSM/covid-cert-cheker.h \
    wicketFSM/main-fsm.h \
    wicketFSM/wicketfsm.h \
    wicketFSM/wicketlocker.h

unix{
    HEADERS += \
    barcode_reader/snapi-barcode-reader.h
    SOURCES += \
    barcode_reader/snapi-barcode-reader.cpp
    LIBS += -lusb-1.0
}
win32{
    HEADERS +=
#    SOURCES +=
}
#CONFIG+=static
QMAKE_CFLAGS += -Ofast
armv5te{
    QMAKE_CFLAGS   += -march=armv5te
    QMAKE_CXXFLAGS += -march=armv5te
}
#QMAKE_CXXFLAGS += -flto
QMAKE_CXXFLAGS += -funroll-loops
QMAKE_CXXFLAGS += -fforce-addr
QMAKE_CXXFLAGS += -Ofast
#QMAKE_CXXFLAGS += -O3

#QMAKE_CFLAGS -= -fno-keep-inline-dllexport

INCLUDEPATH += libs/libusb/include


RESOURCES += \
    picture.qrc
