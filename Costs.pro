#-------------------------------------------------
#
# Project created by QtCreator 2015-09-13T09:21:07
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Costs
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    databaseapi.cpp \
    datastatistics.cpp

HEADERS  += mainwindow.h \
    databaseapi.h \
    datastatistics.h

FORMS    += mainwindow.ui
