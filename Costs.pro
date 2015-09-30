#-------------------------------------------------
#
# Project created by QtCreator 2015-09-13T09:21:07
#
#-------------------------------------------------

QT       += core gui sql concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Costs
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    databaseapi.cpp \
    categoryconfigdialog.cpp \
    myqsqlrelationaltablemodel.cpp \
    paymentmethodsconfigdialog.cpp \
    csvimportdialog.cpp

HEADERS  += mainwindow.h \
    databaseapi.h \
    categoryconfigdialog.h \
    myqsqlrelationaltablemodel.h \
    paymentmethodsconfigdialog.h \
    csvimportdialog.h

FORMS    += mainwindow.ui \
    categoryconfigdialog.ui \
    paymentmethodsconfigdialog.ui \
    csvimportdialog.ui

RESOURCES += \
    icons.qrc

# Icon for Windows
RC_ICONS = costs.ico
