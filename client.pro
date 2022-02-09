CONFIG += ordered

#INCLUDEPATH += $$PWD/common

TEMPLATE = subdirs

SUBDIRS = \
            test-client \

unix{
SUBDIRS += arm-client
}
