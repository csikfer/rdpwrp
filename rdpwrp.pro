#-------------------------------------------------
#
# Project created by QtCreator 2013-08-21T10:24:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = rdpwrp
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    idletimeout.cpp

HEADERS  += dialog.h \
    idletimeout.h

FORMS    += dialog.ui \
    idletimeout.ui

RESOURCES += \
    rdpwrp.qrc
