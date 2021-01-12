#include "kingdom.h"
#include <QPixmap>
#include <QDebug>

Kingdom::Kingdom(int w,int h,QImage::Format fmt):QImage(w,h,fmt)
{

    fill(Qt::transparent);
    QPainter painter(this);
    painter.setRenderHint( QPainter::Antialiasing);
    QPixmap p(":/images/castle.png");
    int origW = p.width();
    qDebug() << "p.width = " << origW;
    float ratio =  origW * 1.0 / w;
    p.setDevicePixelRatio(ratio);
    painter.drawPixmap(0,0,w,h,p);
}
