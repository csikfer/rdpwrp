#-------------------------------------------------
#
# Project created by QtCreator 2013-08-21T10:24:46
#
#-------------------------------------------------

#bison definition
bison.name = Bison
bison.input = BISONSOURCES
bison.output = ${QMAKE_FILE_BASE}_yy.cpp
bison.commands = bison -d -o ${QMAKE_FILE_OUT} -v --report-file=bison_report.txt ${QMAKE_FILE_IN}
bison.clean =
bison.CONFIG += target_predeps
bison.variable_out = SOURCES
QMAKE_EXTRA_COMPILERS += bison

BISONSOURCES += config.yy
DEPENDPATH   += $$TARGETPATH

OTHER_FILES += $$BISONSOURCES
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

 TRANSLATIONS    = rdpwrp_hu.ts \
                   rdpwrp_en.ts

 CODECFORSRC     = UTF-8
