/**
  @file
  @author Stefan Frings
*/
#include <QRegExp>
#include "requesthandler.h"
#include "filelogger.h"
#include <QFile>
#include "./app/Module_RandomName.h"
#include "./app/public.h"
#include "./app/gen3dmappoints.h"
#include <QDateTime>
#include <QThread>
#include <QDir>
#include "app/threadstat.h"
/** Logger class */
long long g_mapGenTimes = 0;
long long g_visitCnt = 0;
extern FileLogger* logger;
static ThreadStat * g_statThread = NULL;
static QByteArray g_ba_index_html;
//static QByteArray g_ba_randomMap_html;
//static QByteArray g_ba_randomName_html;
//static QByteArray g_ba_randomNickname_html;
static QByteArray g_ba_favicon_ico;
static QByteArray g_ba_randMap_png;
static QByteArray g_ba_logo_png;
RequestHandler::RequestHandler(QObject* parent)
    :HttpRequestHandler(parent)
{
    if(g_statThread  == NULL){
        g_statThread = new ThreadStat;
        g_statThread->start();
    }
}
static void delFilesSecondsAgo(int seconds = 5){
    QDir dir(g_path_mapResult);
    dir.setFilter(QDir::Files);
    QFileInfoList fiList = dir.entryInfoList();
    QDateTime dt;
    long long curSec = QDateTime::currentSecsSinceEpoch();
    long long fileSec = 0;
    QStringList delList;
    for(int i = 0;i<fiList.size();++i){
        QFileInfo & fi = fiList[i];
        //qDebug() << "fi.fileName = " << fi.fileName(); //map_8.svg
        dt = fi.created();
        fileSec = dt.toSecsSinceEpoch();
        if(curSec - fileSec > seconds){
            delList.push_back(fi.absoluteFilePath());
        }
    }
    for(int i = 0;i<delList.size();++i){
        QFile f(delList[i]);
        f.remove();
    }
}
static QString getIDFromSvgPath(const QString & path){
    QStringList tmpList = path.split("map_");
    if(tmpList.size() < 2) return "";
    QString str = tmpList.at(1);
    if(str.contains(".svg")){
        return str.split(".svg")[0];
    }
    return "";
}
void RequestHandler::service(HttpRequest& request, HttpResponse& response)
{
    QString path=request.getPath().data();
    printf("path:%s\n",path.toStdString().c_str());
    if(path == "/" || path == "/index.html"){
        g_visitCnt++;
        if(g_ba_index_html.isEmpty()){
            QString name = g_projectAPath + "/index.html";
            QFile f(name);
            bool ok = f.open(QIODevice::ReadOnly);
            if(ok){
                g_ba_index_html = f.readAll();
                f.close();
            }
        }
        // Set a response header
        response.setHeader("Content-Type", "text/html; charset=utf-8");
        // Return a simple HTML document
        response.write(g_ba_index_html,true);

    }
    else if(path.startsWith("/images/")){
        if(path.contains("logo")){
            if(g_ba_logo_png.isEmpty()){
                QString name = g_projectAPath + path;
                QFile f(name);
                bool ok = f.open(QIODevice::ReadOnly);
                if(ok){
                    g_ba_logo_png = f.readAll();
                    f.close();
                }
            }
            response.setHeader("Content-Type", "image/png");
            response.write(g_ba_logo_png,true);
        }
        else if(path.contains("randMap")){
            if(g_ba_randMap_png.isEmpty()){
                QString name = g_projectAPath + path;
                QFile f(name);
                bool ok = f.open(QIODevice::ReadOnly);
                if(ok){
                    g_ba_randMap_png = f.readAll();
                    f.close();
                }
            }
            response.setHeader("Content-Type", "image/png");
            response.write(g_ba_randMap_png,true);
        }

    }
    else if(path == "/favicon.ico"){
        if(g_ba_favicon_ico.isEmpty()){
            QString name = g_projectAPath + path;
            QFile f(name);
            bool ok = f.open(QIODevice::ReadOnly);
            if(ok){
                g_ba_favicon_ico = f.readAll();
                f.close();
            }
        }
        response.setHeader("Content-Type", "image/png");
        response.write(g_ba_favicon_ico,true);

    }
    else if(path.startsWith("/js/") || path.startsWith("/css/")){
        QString name = g_projectAPath + path;
        QFile f(name);
        bool ok = f.open(QIODevice::ReadOnly);
        if(ok){
            QByteArray ba = f.readAll();
            f.close();
            //response.setHeader("Content-Type", "text/plain; charset=utf-8");
            response.write(ba,true);
        }
    }
    else if(path == "/randNickname.html" || path == "/randMap.html" ||
            path == "/randName.html" ){
        QString name = g_projectAPath + path;
        QFile f(name);
        bool ok = f.open(QIODevice::ReadOnly);
        if(ok){
            QByteArray ba = f.readAll();
            f.close();
            response.setHeader("Content-Type", "text/html; charset=utf-8");
            response.write(ba,true);
        }
    }
    else if(path.startsWith("/getName/")){
        QString str = path.split("/getName/")[1];
        Module_RandomName::FirstNameType fnt = Module_RandomName::FNT_Single;
        if(str.contains("single_")){
            str = str.split("single_").at(1);
        }
        else if(str.contains("double_")){
            str = str.split("double_").at(1);
            fnt = Module_RandomName::FNT_Double;
        }
        else return;

        if(str.contains("_and_")){
            QString part1 = str.split("_and_")[0];
            QString part2 = str.split("_and_")[1];
            Module_RandomName::NameElement ne1,ne2;
            if(part1 == "n") ne1 = Module_RandomName::NS_N;
            else if(part1 == "adj") ne1 = Module_RandomName::NS_Adj;
            else if(part1 == "all") ne1 = Module_RandomName::NS_All;
            else if(part1 == "v") ne1 = Module_RandomName::NS_V;
            else ne1 = Module_RandomName::NS_O;
            if(part2 == "n") ne2 = Module_RandomName::NS_N;
            else if(part2 == "adj") ne2 = Module_RandomName::NS_Adj;
            else if(part2 == "all") ne2 = Module_RandomName::NS_All;
            else if(part2 == "v") ne2 = Module_RandomName::NS_V;
            else if(part2 == "o") ne2 = Module_RandomName::NS_O;
            else  ne2 = Module_RandomName::NS_NULL;

            Module_RandomName & mr = Module_RandomName::getInstance();
            QStringList resList;
            mr.getLongNames(resList,ne1,ne2,10,fnt);
            QString ret = QString("RN:") + resList.join("-");
            // Set a response header
            response.setHeader("Content-Type", "text/plain; charset=utf-8");
            // Return a simple HTML document
            response.write(ret.toUtf8(),true);
        }
    }
    else if(path.startsWith("/getNickname")){
        QStringList resList;
        Module_RandomName & mr = Module_RandomName::getInstance();
        mr.getNickName(resList,10);
        QString ret = QString("RNN:") + resList.join("-");
        // Set a response header
        response.setHeader("Content-Type", "text/plain; charset=utf-8");
        // Return a simple HTML document
        response.write(ret.toUtf8(),true);
    }
    else if(path.startsWith("/getEnglishName/")){
        QStringList resList;
        Module_RandomName & mr = Module_RandomName::getInstance();
        if(path.contains("boy")){
            mr.getMaleNames(resList,10);
        }
        else{
            mr.getFemaleNames(resList,10);
        }
        QString ret = QString("REN:") + resList.join("-");
        // Set a response header
        response.setHeader("Content-Type", "text/plain; charset=utf-8");
        // Return a simple HTML document
        response.write(ret.toUtf8(),true);
    }
    else if(path.startsWith("/getContinent")){
        // /getContinent_fill or /getContinent_noFill
        // /getContinent_fill_singleLand (fill/noFill,singleLand/multiLand)
        Gen3DMapPoints * gm = new Gen3DMapPoints;
        long long idx = ++g_mapGenTimes;
        bool fillColor = path.contains("_fill");
        bool singleLand = path.contains("singleLand");

        QString resPath = gm->generateAMap(idx,fillColor,singleLand);
        //gm->saveMeshData();
        delete gm;
        QFile f(resPath);
        bool ok = f.open(QIODevice::ReadOnly);
        if(ok){
            QByteArray ba = f.readAll();
            f.close();
            QString data = QString::fromUtf8(ba);

            QString str_id = "<id=" + getIDFromSvgPath(resPath) + ">";
            QString str_viewBox = "<viewbox=4000*2000>";

            data.replace("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>",str_id + str_viewBox);
            data.replace("<title>Qt SVG Document</title>","fantasy world");
            //<id=3><viewbox=4000*2000>
            //static QRegExp rx("<svg width=.*height=.* ");
            //rx.setMinimal(true);
            //data.replace(rx,"<svg viewbox=\"0 0 4000 2000\" ");

            ba = data.toUtf8();
            response.setHeader("Content-Type", "text/plain; charset=utf-8");
            response.write(ba,true);
            printf("task finished\n");
        }
        delFilesSecondsAgo(5);
    }
    else if( path.startsWith("/getPngByID_") ){
        QString str_id = path.split("/getPngByID_").at(1);
        QString path = g_path_mapResult + "map_" + str_id + ".png";
        QFile f(path);
        bool ok = f.open(QIODevice::ReadOnly);
        if(ok){
            QByteArray ba = f.readAll();
            f.close();
            response.setHeader("Content-Type", "image/png");
            response.write(ba,true);
            printf("png file sent!\n");
        }
    }
    else{
        return;
    }
    // Clear the log buffer
    if (logger)
    {
       logger->clear();
    }
}
