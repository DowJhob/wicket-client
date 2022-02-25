CONFIG += ordered

#INCLUDEPATH += $$PWD/common

#include(common/common.pri)

TEMPLATE = subdirs

SUBDIRS = \
            test-client \

unix{
SUBDIRS += arm-client
}
