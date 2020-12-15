/**
  @file
  @author Stefan Frings
*/

#include "static.h"
#include "startup.h"
#include "filelogger.h"
#include "requesthandler.h"
#include <QDir>
#include <QFile>
#include "./app/public.h"

/** Name of this application */
#define APPNAME "whiterunStore"

/** Publisher of this application */
#define ORGANISATION "WhiterunCommunity"

/** Short description of the Windows service */
#define DESCRIPTION "GroceryStoreInWhiterunCity"

/** Logger class */
FileLogger* logger;

/** Search the configuration file */

QString searchConfigFile()
{
    QString binDir=QCoreApplication::applicationDirPath();
    QString appName=QCoreApplication::applicationName();
    qDebug() << "appName = " << appName;

    QFile file(g_projectAPath+"/whiterunStore.ini");
    if (file.exists())
    {
        // found
        QString fileName=QDir(file.fileName()).canonicalPath();
        qDebug("Using config file %s",qPrintable(fileName));
        return fileName;
    }


    qFatal("Cannot find config file %s",qPrintable("whiterunStore.ini"));
    return 0;
}

void Startup::start()
{
    // Initialize the core application
    QCoreApplication* app = application();
    app->setApplicationName(APPNAME);
    app->setOrganizationName(ORGANISATION);

    // Find the configuration file
    QString configFileName=searchConfigFile();

    // Configure logging into a file
    QSettings* logSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    logSettings->beginGroup("logging");
    logger=new FileLogger(logSettings,10000,app);
    logger->installMsgHandler();

    // Configure and start the TCP listener
    qDebug("ServiceHelper: Starting service");
    QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    listenerSettings->beginGroup("listener");
    listener=new HttpListener(listenerSettings,new RequestHandler(app),app);

    qWarning("Startup: Service has started");
}

void Startup::stop()
{
    // Note that this method is only called when the application exits itself.
    // It is not called when you close the window, press Ctrl-C or send a kill signal.

    delete listener;
    qWarning("Startup: Service has been stopped");
}


Startup::Startup(int argc, char *argv[])
    : QtService<QCoreApplication>(argc, argv, APPNAME)
{
    setServiceDescription(DESCRIPTION);
    setStartupType(QtServiceController::AutoStartup);
}



