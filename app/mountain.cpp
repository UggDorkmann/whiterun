#include "mountain.h"
#include <QPixmap>
#include <QDebug>
#include <QGuiApplication>
static bool g_hasGui = false;
Mountain::Mountain(int w,int h,QImage::Format fmt):QImage(w,h,fmt)
{
    if(!g_hasGui){
        g_hasGui = true;
        int argc = 1;
        char appname[10] = {'t','m','p','G','u','i'};
        char * arr[1];
        arr[0] = appname;
        static QGuiApplication g_gui(argc,arr);
    }
    fill(Qt::transparent);

    QPainter painter(this);
    painter.setRenderHint( QPainter::Antialiasing);
    QPixmap p(":/images/hill.png");
    int origW = p.width();
    float ratio =  origW * 1.0 / w;
    p.setDevicePixelRatio(ratio);
    painter.drawPixmap(0,0,w,h,p);
}
