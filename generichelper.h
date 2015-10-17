#ifndef GENERICHELPER_H
#define GENERICHELPER_H

#include <QObject>

class GenericHelper
{
public:
    static QString getAppName();
    static QString getCompanyName();

    static QSize getSettingMainWindowSize();
    static void setSettingMainWindowSize(QSize size);

    static QPoint getSettingMainWindowPos();
    static void setSettingMainWindowPos(QPoint pos);

    static qint32 getSettingCurrentTab();
    static void setSettingCurrentTab(qint32 tab);

    static bool getSettingDatabaseIsOpen();
    static void setSettingDatabaseIsOpen(bool isOpen);

    static QString getSettingDatabaseFileName();
    static void setSettingDatabaseFileName(QString dbfilename);

    static void setSettingFileDialogPath(QString filepath);
    static QString getSettingFileDialogPath();

    static void setSettingCSVImportDialogPath(QString filepath);
    static QString getSettingCSVImportDialogPath();
};

#endif // GENERICHELPER_H
