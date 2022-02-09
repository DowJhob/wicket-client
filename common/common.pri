CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

QT += core network
QT += gui
QT += widgets
QT += serialport

include(../git-ver.pri)

common = $$PWD/

SOURCES += \
            $$PWD/controller.cpp \
            $$PWD/network.cpp \
            $$PWD/widgets/mainStackedWgt.cpp \
            $$PWD/widgets/showCertInfo.cpp \
            $$PWD/widgets/showStateWgt.cpp

HEADERS += \
            $$PWD/common_types.h \
            $$PWD/command.h \
            $$PWD/controller.h \
            $$PWD/network.h \
            $$PWD/widgets/mainStackedWgt.h \
            $$PWD/widgets/showCertInfo.h \
            $$PWD/widgets/showStateWgt.h

RESOURCES += \
    $$PWD/picture.qrc
