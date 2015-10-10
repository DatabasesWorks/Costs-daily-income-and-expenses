#-------------------------------------------------
#
# Project created by QtCreator 2015-09-13T09:21:07
#
#-------------------------------------------------

QT       += core gui sql concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Costs
TEMPLATE = app

VERSION_MAJOR = 0
VERSION_MINOR = 0
VERSION_PATCH = 3
VERSION_BUILD = 0

# Get build from file
VERSION_BUILD_FILE = $$cat($$OUT_PWD\buildnr.txt)
greaterThan(VERSION_BUILD_FILE, 0) {
	VERSION_BUILD = $$format_number($${VERSION_BUILD_FILE}, ibase=10 width=1 zeropad)
}

QMAKE_TARGET_COMPANY = "AbyleDotOrg"
QMAKE_TARGET_PRODUCT = "Costs"
QMAKE_TARGET_DESCRIPTION = "Costs"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2015 AbyleDotOrg"

DEFINES += \
APP_COMPANY=\"\\\"$$QMAKE_TARGET_COMPANY\\\"\" \
APP_PRODUCT=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\" \
APP_DESCRIPTION=\"\\\"$$QMAKE_TARGET_DESCRIPTION\\\"\" \
APP_COPYRIGHT=\"\\\"$$QMAKE_TARGET_COPYRIGHT\\\"\" \
APP_NAME=\\\"$$TARGET\\\"

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_PATCH=$$VERSION_PATCH"\
       "VERSION_BUILD=$$VERSION_BUILD"

#Target version
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}.$${VERSION_BUILD}

SOURCES += main.cpp\
        mainwindow.cpp \
    databaseapi.cpp \
    categoryconfigdialog.cpp \
    myqsqlrelationaltablemodel.cpp \
    paymentmethodsconfigdialog.cpp \
    csvimportdialog.cpp \
    myplots.cpp

HEADERS  += mainwindow.h \
    databaseapi.h \
    categoryconfigdialog.h \
    myqsqlrelationaltablemodel.h \
    paymentmethodsconfigdialog.h \
    csvimportdialog.h \
    myplots.h

FORMS    += mainwindow.ui \
    categoryconfigdialog.ui \
    paymentmethodsconfigdialog.ui \
    csvimportdialog.ui

RESOURCES += \
    icons.qrc

# Icon for Windows
RC_ICONS = costs.ico

DISTFILES += \
    README.md
