# // ******************************************************
# // * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
# // * All Rights reserved
# // * File: templ5.pro
# // * created 2014/07/01 by rbehm
# // ******************************************************

TEMPLATE = app
include( $$[PRTEMPLATE]/project.pri )
include( $$[PRTEMPLATE]/lib/utils/stdicons.pri )
include( $$[PRTEMPLATE]/lib/utils/toolbarspacer.pri )
include( $$[PRTEMPLATE]/lib/utils/ledicon/ledicon.pri )
include( ./serialport/ipccomm.pri )
#include( $$[PRTEMPLATE]/qcustomplot.pri )
DEFINES += QCUSTOMPLOT_USE_LIBRARY QCUSTOM_PLOT

# select which ones are needed
QT += core gui widgets
QT += serialport printsupport

DESTDIR=./bin

HEADERS += \
    mainwindow.h \
    propscope.h \
    channelsetup.h \
    qcustomplot.h \
    scopescreen.h \
    scopesetup.h \
    triggersetup.h \
    configdata.h \
    analogdata.h \
    selectbox.h \
    triggerdata.h \
    scopecursor.h \
    channelgraph.h \
    utils.h \
    measurements.h \
    datalabel.h \
    pointsmodel.h \
    pointtableview.h \
    valuedelegate.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    propscope.cpp \
    channelsetup.cpp \
    qcustomplot.cpp \
    scopescreen.cpp \
    scopesetup.cpp \
    triggersetup.cpp \
    configdata.cpp \
    selectbox.cpp \
    triggerdata.cpp \
    scopecursor.cpp \
    channelgraph.cpp \
    utils.cpp \
    measurements.cpp \
    datalabel.cpp \
    pointsmodel.cpp \
    pointtableview.cpp \
    valuedelegate.cpp

RESOURCES += \
    icons.qrc
OTHER_FILES += styles.css

DISTFILES += \
    ../LICENSE \
    ../README.md

