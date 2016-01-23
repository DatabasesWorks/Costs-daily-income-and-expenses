#include "generichelper.h"

#include <QSettings>
#include <QSize>
#include <QPoint>
#include <QString>
#include <QFile>
#include <QDebug>

QString GenericHelper::getAppName()
{
    return QString(APP_NAME);
}

QString GenericHelper::getCompanyName()
{
    return QString(APP_COMPANY);
}


QSize GenericHelper::getSettingMainWindowSize()
{
    QSize size;

    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("MainWindow");
    size = settings.value("size", QSize(400, 400)).toSize();
    settings.endGroup();

    return size;
}

void GenericHelper::setSettingMainWindowSize(QSize size)
{
    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("MainWindow");
    settings.setValue("size", size);
    settings.endGroup();
}

QPoint GenericHelper::getSettingMainWindowPos()
{
    QPoint pos;

    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("MainWindow");
    pos = settings.value("pos", QPoint(200, 200)).toPoint();
    settings.endGroup();

    return pos;
}

void GenericHelper::setSettingMainWindowPos(QPoint pos)
{
    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("MainWindow");
    settings.setValue("pos", pos);
    settings.endGroup();
}

qint32 GenericHelper::getSettingCurrentTab()
{
    qint32 tab;

    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("MainWindow");
    tab = settings.value("currentTab", 0).toInt();
    settings.endGroup();

    return tab;
}

void GenericHelper::setSettingCurrentTab(qint32 tab)
{
    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("MainWindow");
    settings.setValue("currentTab", tab);
    settings.endGroup();
}

bool GenericHelper::getSettingDatabaseIsOpen()
{
    bool isOpen;

    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("Database");
    isOpen = settings.value("isOpen").toBool();
    settings.endGroup();

    return isOpen;
}

void GenericHelper::setSettingDatabaseIsOpen(bool isOpen)
{
    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("Database");
    settings.setValue("isOpen", isOpen);
    settings.endGroup();
}

QString GenericHelper::getSettingDatabaseFileName()
{
    QString dbfilename;

    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("Database");
    dbfilename = settings.value("dbfilename").toString();
    settings.endGroup();

    return dbfilename;
}

void GenericHelper::setSettingDatabaseFileName(QString dbfilename)
{
    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("Database");
    settings.setValue("dbfilename", dbfilename);
    settings.endGroup();
}


void GenericHelper::setSettingFileDialogPath(QString filepath)
{
    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("MainWindow");
    settings.setValue("oldFileDialogPath", filepath);
    settings.endGroup();
}

QString GenericHelper::getSettingFileDialogPath()
{
    QString filepath;

    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("MainWindow");
    filepath = settings.value("oldFileDialogPath", "").toString();
    settings.endGroup();

    return filepath;
}

void GenericHelper::setSettingCSVImportDialogPath(QString filepath)
{
    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("MainWindow");
    settings.setValue("oldCSVImportDialogPath", filepath);
    settings.endGroup();
}

QString GenericHelper::getSettingCSVImportDialogPath()
{
    QString filepath;

    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("MainWindow");
    filepath = settings.value("oldCSVImportDialogPath", "").toString();
    settings.endGroup();

    return filepath;
}

bool GenericHelper::getSettingBackupDatabase()
{
    bool backup;

    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    settings.beginGroup("MainWindow");
    backup = settings.value("backupDatabase", true).toBool();
    settings.endGroup();

    return backup;
}

bool GenericHelper::copyFile(QFile inFile, QFile outFile, bool overwrite)
{
    if ( ! outFile.exists() ) {
        return inFile.copy(outFile.fileName());
    } else if (overwrite) {
        if( outFile.remove() ) {
            return inFile.copy(outFile.fileName());
        }
    } else {
        return false;
    }

    return false;
}

bool GenericHelper::copyFile(QString in, QString out, bool overwrite)
{
    QFile inFile(in);
    QFile outFile(out);
    if (! outFile.exists() ) {
        return inFile.copy(outFile.fileName());
    } else if (overwrite) {
        if( outFile.remove() ) {
            return inFile.copy(outFile.fileName());
        }
    } else {
        return false;
    }

    return false;
}
