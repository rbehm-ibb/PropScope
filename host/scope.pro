# // ******************************************************
# // * copyright (C) 2015 by Reinhardt Behm/rbehm@hushmail.com
# // * All Rights reserved
# // * File: templ5.pro
# // * created 2014/07/01 by rbehm
# // ******************************************************

TEMPLATE = app
include( ./templateQt5/project.pri )
include( ./templateQt5/lib/utils/stdicons.pri )
include( ./templateQt5/lib/utils/toolbarspacer.pri )
include( ./templateQt5/lib/utils/ledicon/ledicon.pri )
include( ./serialport/ipccomm.pri )
include( ./templateQt5/qcustomplot.pri )

# select which ones are needed
QT += core gui widgets
QT += serialport

DESTDIR=./bin

HEADERS += \
    mainwindow.h \
    propscope.h \
    channelsetup.h \
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

