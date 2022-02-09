include(../common/common.pri)

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
        barcode_reader/serial.cpp \
        main.cpp \
        nikiret.cpp \
        controller.cpp \
#        wicketFSM/w0.nestedstate.cpp \
#        wicketFSM/w1.main-fsm.cpp \
#        wicketFSM/w2.wicketlocker.cpp \
#        wicketFSM/w3.countercheck.cpp \
#        wicketFSM/w3.covid-cert-cheker.cpp \
#        wicketFSM/wicketfsm.cpp

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
    barcode_reader/barcode_msg.h \
    barcode_reader/libusb-wrapper.h \
    barcode_reader/serial.h \
    libs/libusb/include/libusb.h \
    nikiret.h \
    controller.h \
#    updater.h \
#    wicketFSM/w0.nestedstate.h \
#    wicketFSM/w1.main-fsm.h \
#    wicketFSM/w2.wicketlocker.h \
#    wicketFSM/w3.countercheck.h \
#    wicketFSM/w3.covid-cert-cheker.h \
#    wicketFSM/wicketfsm.h

unix{
    HEADERS += \
    barcode_reader/snapi-barcode-reader.h
    SOURCES += \
    barcode_reader/snapi-barcode-reader.cpp
    LIBS += -lusb-1.0

    armv5te{
        QMAKE_CFLAGS   += -march=armv5te
        QMAKE_CXXFLAGS += -march=armv5te
    }
}
win32{
    HEADERS +=
#    SOURCES +=
}
CONFIG(release, release|debug){
                            QMAKE_CXXFLAGS  += -flto -funroll-loops
                            CONFIG += -static
                    }

#CONFIG+=static
QMAKE_CFLAGS += -Ofast
#QMAKE_CXXFLAGS += -flto
QMAKE_CXXFLAGS += -fforce-addr
QMAKE_CXXFLAGS += -Ofast
#QMAKE_CXXFLAGS += -O3

#QMAKE_CFLAGS -= -fno-keep-inline-dllexport

INCLUDEPATH += libs/libusb/include
