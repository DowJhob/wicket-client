#QT -= gui
QT += core network
QT += gui
QT += widgets
QT += serialport

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
        async_threaded_reader.cpp \
        main.cpp

INSTALLS += target
target.path = target.path = /usr
target.files = rd

HEADERS += common_types.h \
#    NFC.h \
    async_threaded_reader.h \
#controller.h \
#    barcode_qt.h \
    controller_2.h \
    network.h \
    nikiret.h \
    picture2.h \
    updater.h \
    wicketFSM/wicketfsm.h \
    wicketFSM/wicketlocker.h


#CONFIG+=static
#QMAKE_CXXFLAGS += -Wno-psabi
##QMAKE_CFLAGS += -O3
#QMAKE_CFLAGS +=-march=armv5te
QMAKE_CFLAGS += -Ofast
QMAKE_CFLAGS +=-march=armv5te

QMAKE_CXXFLAGS += -march=armv5te
#QMAKE_CXXFLAGS += -flto
QMAKE_CXXFLAGS += -funroll-loops
QMAKE_CXXFLAGS += -fforce-addr
QMAKE_CXXFLAGS += -Ofast
#QMAKE_CXXFLAGS += -O3

#QMAKE_CFLAGS -= -fno-keep-inline-dllexport
#FORMS +=
#LIBS += -lusb
#LIBS += -L$$PWD/./libs
#LIBS +=          libusb-1.0.so
LIBS += -Llibs/libusb -lusb-1.0
INCLUDEPATH += libs/libusb/include
#PRE_TARGETDEPS+= libusb-1.0.so
#INCLUDEPATH += ./
#DEPENDPATH += ./
RESOURCES += \
    picture.qrc
