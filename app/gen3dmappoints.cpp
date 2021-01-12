#include "gen3dmappoints.h"
#include <QFile>
#include<stdlib.h>
#include <stdio.h>
#include <QDebug>
#include <ctime>
#include <math.h>
#include <QGraphicsScene>
#include "public.h"
#include <QPainter>
#include <QSvgGenerator>
#include <QMutex>
#include <QDir>
#include <QDateTime>
#include <QtMath>
#include "kingdom.h"
#include "mountain.h"
void Gen3DMapPoints::getCenter(Point & center,float x1,float y1,float x2,float y2,const Point * nxt){
    static QString log;
    static const int bigK = 10000;
    static const float gap = 0.00001;
    log.clear();
    if(fabs(nxt->m_y - y1) < 1){// cur--nxt is horizontal
        log += "cur--nxt is H,";
        if(fabs(nxt->m_x - x2) < 1) // nxt--far is vertical
        {
            center.m_x = x1;
            center.m_y = y2;
            log += "nxt--far is V,center:" + center.str();
        }
        else{//normal

            //float k = -1 / ( (y2 - nxt->m_y) / (x2 - nxt->m_x) );
            float k = 1;
            if(fabs(y2 - nxt->m_y) < gap){
                k = bigK;
            }
            else{
                k = -1 * (x2 - nxt->m_x) / (y2 - nxt->m_y) ;
            }
            //y - y2 = k(x - x2);
            float y = k * (x1 - x2) + y2;
            center.m_x = x1;
            center.m_y = y;
            log += "nxt--far is Normal,k = " + QString::number(k) + ",center:" + center.str();
        }
    }
    else if(fabs(nxt->m_x - x1) < 1){ // cur--nxt is vertical
        log += "cur--nxt is V,";
        if(fabs(y2 - nxt->m_y) < 1){// nxt--far is horizontal

            center.m_x = x2;
            center.m_y = y1;
            log += "nxt--far is H,center:" + center.str();
        }
        else{//normal
            float k = 1;
            if(fabs(y2 - nxt->m_y) < gap){
                k = bigK;
            }
            else{
                k = (x2 - nxt->m_x) / (y2 - nxt->m_y) * -1 ;
            }
            if(fabs(k) < gap) k = gap;
            float x = (y1 - y2) / k + x2;
            center.m_x = x;
            center.m_y = y1;
            log += "nxt--far is Normal,k = " + QString::number(k) + ",center:" + center.str();
        }
    }
    else{
        log += "cur--nxt is Normal,";
        if(fabs(nxt->m_x - x2) < 1) // nxt--far is vertical
        {
            float k = 1;
            if(fabs(y1 - nxt->m_y) < gap){
                k = bigK;
            }
            else k = (x1 - nxt->m_x) / (y1 - nxt->m_y) * -1 ;
            if(fabs(k) < gap) k = gap;
            float x = (y2 - y1) / k + x1;
            center.m_x = x;
            center.m_y = y2;
            log += "nxt--far is V," + QString::number(k) + center.str();
        }
        else if(fabs(nxt->m_y - y2) < 1){ // nxt--far is horizontal
            float k = 1;
            if(fabs(y1 - nxt->m_y) < gap){
                k = bigK;
            }
            else
                k = -1* (x1 - nxt->m_x) / (y1 - nxt->m_y) ;
            float y = k*(x2 - x1) + y1;
            center.m_x = x2;
            center.m_y = y;
            log += "nxt--far is H,k = " + QString::number(k) + center.str();
        }
        else{ //most scenario
            float k1 = 1;
            float k2 = 1;
            if(fabs(y1 - nxt->m_y) < gap){
                k1 = bigK;
            }
            else k1 = -1 * (x1 - nxt->m_x) / (y1 - nxt->m_y);
            if(fabs(y2 - nxt->m_y) < gap){
                k2 = bigK;
            }
            else k2 = -1 * (x2 - nxt->m_x) / (y2 - nxt->m_y);
            float deltaK = k1 - k2;
            if(fabs(k1 - k2) < gap) deltaK = deltaK > 0 ? gap : gap * -1;
            float x = (k1 * x1 - k2 * x2 + y2 - y1) / deltaK;
            float y = y1 + k1 * (x - x1);
            center.m_x = x;
            center.m_y = y;
            log += "nxt-far is Normal,k1 = " +QString::number(k1) + ",k2 = " + QString::number(k2) +
                    ",center:" + center.str();
        }
    }
    if(center.m_x > sqrt(__FLT_MAX__) ){
        qDebug() << "weird scinario:" << log;
        //qDebug() << "x2:" << x2 <<",nxt.m_x = " << nxt->m_x << ",y2 = " << y2 << ",nxt.m_y = " << nxt->m_y;
    }
}
float Gen3DMapPoints::getAngle(Point & center,float arcX,float arcY){
    float ang;
    static const float pi = 3.1415926;
    float cx = center.m_x;
    float cy = center.m_y;
    if(fabs(cx - arcX) < 1){
        ang = 90;
        if(cy < arcY) ang *= -1;
    }
    else{
        ang = atan((cy - arcY) / (cx - arcX)) * 180 / pi;
        if(ang > 0){ //二 四 象限,因为y轴是向下的
            if(arcX < cx){ //第二象限
                ang = 180 - ang;
            }
            else{ //第四象限
                ang *= -1;
            }
        }
        else if(ang < 0){
            if(arcX < cx){//第三象限
                ang = -180 - ang;
            }
            else{//第一象限
                ang *= -1;
            }
        }
        else{
            if(arcX < cx){
                ang = 180;
            }
            else{
                ang = 0;
            }
        }
    }
    return ang;
}
void Gen3DMapPoints::extend(Point* cur,Point* nxt,Point* far,QPainterPath & pp){
    //这个函数做两件事,lineTo 和 arcTo
    float len1 = sqrt((cur->m_x - nxt->m_x) * (cur->m_x - nxt->m_x) +
                      (cur->m_y - nxt->m_y) * (cur->m_y - nxt->m_y));
    float len2 = sqrt((far->m_x - nxt->m_x) * (far->m_x - nxt->m_x) +
                      (far->m_y - nxt->m_y) * (far->m_y - nxt->m_y));
    float ratio1,ratio2;
    if(len1 > len2){
        ratio2 = 0.25;
        ratio1 = 0.25 * len2 / len1;
    }
    else{
        ratio1 = 0.25;
        ratio2 = 0.25 * len1 / len2;
    }

    float arcStartX = cur->m_x + (1 - ratio1) * (nxt->m_x - cur->m_x);
    float arcStartY = cur->m_y + (1 - ratio1) * (nxt->m_y - cur->m_y);

    pp.lineTo(QPointF(arcStartX,arcStartY));

    float arcEndX = nxt->m_x + ratio2 * (far->m_x - nxt->m_x);
    float arcEndY = nxt->m_y + ratio2 * (far->m_y - nxt->m_y);

    Point center;
    getCenter(center,arcStartX,arcStartY,arcEndX,arcEndY,nxt);

    float r = sqrt( (center.m_x - arcStartX) * (center.m_x - arcStartX) +
                    (center.m_y - arcStartY) * (center.m_y - arcStartY));
    if(r > 100000){
        qDebug() << "r > 100000 , r=" <<r << ",center:" << center.str() << "cos = " << getCos(nxt,cur,far);
    }
    QRect rct(0,0,r*2,r*2);
    rct.moveCenter(QPoint(center.m_x,center.m_y));


    float startAngle = getAngle(center,arcStartX,arcStartY);

    float endAngle = getAngle(center,arcEndX,arcEndY);
    float spanAngle = endAngle - startAngle;
    while(spanAngle > 180){
        spanAngle -= 360;
    }
    while(spanAngle < -180){
        spanAngle += 360;
    }
    pp.arcTo(rct,startAngle,spanAngle);
}
void Gen3DMapPoints::getFilletPath(const QList<Point*> & path,QPainterPath & pp){
    if(path.length() <=2 ) return;
    bool headTailLap = false;
    if(path[0] == path[path.length() - 1]) headTailLap = true;
    Point * cur = NULL;
    Point * nxt = NULL;
    Point * far = NULL;
    //pp.clear();
    /* 把路径的起点设置为第一个截点距离第二个截点中点的位置 */
    float startx = path[0]->m_x + 0.5 * (path[1]->m_x - path[0]->m_x);
    float starty = path[0]->m_y + 0.5 * (path[1]->m_y - path[0]->m_y);
    pp.moveTo(QPointF(startx,starty));

    for(int i = 0;i<path.size();++i){
        if(i == path.length() - 2){
            if(headTailLap){
                cur = path[i];
                nxt = path[0];
                far = path[1];
            }
            else{
                cur = path[i];
                nxt = path[i+1];
                far = path[0];
            }
        }
        else if(i == path.length() - 1){
            if(headTailLap){
                break;
            }
            else{
                cur = path[i];
                nxt = path[0];
                far = path[1];
            }
        }
        else{
            cur = path[i];
            nxt = path[i+1];
            far = path[i+2];
        }
        float cos = getCos(nxt,cur,far);
        static const float _175 = cosf(17.5*3.1415926/18);
        if(cos < _175){
            pp.lineTo(QPointF(nxt->m_x,nxt->m_y));//比175度还大的钝角就不倒角了
        }
        else{
            extend(cur,nxt,far,pp);
        }

    }
    pp.lineTo(QPointF(startx,starty));
}

/* 删除单个贝塞尔控制点链表 */
static void delSingleBazierControlList(QList<QList<Point*> > & controlList){
    for(int i = 0;i< controlList.size();++i){
        QList<Point*> & tmpList = controlList[i];
        for(int j = 0;j<tmpList.size();++j){
            Point* p = tmpList[j];
            if(p) delete p;
        }
    }
    controlList.clear();
}
/* 根据pathList中的单个路径的长度排序 */
static void sort(QList<QList<Point*> > & pathList){
    QList<QList<Point*> > sortList;
    int origLen = pathList.size();
    for(int i = 0; i < origLen;++i){
        int idx = 0;
        int len = pathList.size();
        int maxLen = 0;
        for(int j = 0;j < len;++j){
            if(maxLen < pathList[j].size()){
                maxLen = pathList[j].size();
                idx = j;
            }
        }
        sortList.push_back(pathList[idx]);
        pathList.removeAt(idx);
    }
    pathList = sortList;
}
/* 删除截点链表,释放内存 */
static void delPointList(QList<Point*> & list){
    if(list.size() <= 0) return;
    for(int i = 0;i<list.size();++i){
        Point* p = list[i];
        if(p){
            delete p;
        }
    }
    list.clear();
}
static void delPointList(QList<QList<Point*> > & list){
    for(int i = 0;i<list.size();++i){
        QList<Point*> & path = list[i];
        for(int j = 0;j< path.size();++j){
            if(!path[j]) continue;
            delete path[j];
        }
    }
    list.clear();
}
/* 删除贝塞尔双控制点链表 */
static void delDoubleBasierControlList(QList<QList<Controls > > & controlList ){
    for(int i = 0;i<controlList.size();++i){
        QList<Controls > & list = controlList[i];
        for(int j = 0;j<list.size();++j){
            Controls & pair = list[j];
            delete pair.m_c1;
            delete pair.m_c2;
        }
        list.clear();
    }
    controlList.clear();
}
/* 根据三个点形成的三角形,获取cur这个点的余弦值 */
double Gen3DMapPoints::getCos(const Point* mid,const Point* prv,const Point* nxt){
    if(!mid || !prv || !nxt) return 0;
    if(mid == prv || mid == nxt || prv == nxt) return 0;
    if(*mid == *prv || *mid == *nxt || *prv == *nxt) return 0;
    double a = sqrt((prv->m_x - nxt->m_x)*(prv->m_x - nxt->m_x) + (prv->m_y - nxt->m_y)*(prv->m_y - nxt->m_y));
    double b = sqrt((prv->m_x - mid->m_x)*(prv->m_x - mid->m_x) + (prv->m_y - mid->m_y)*(prv->m_y - mid->m_y));
    double c = sqrt((mid->m_x - nxt->m_x)*(mid->m_x - nxt->m_x) + (mid->m_y - nxt->m_y)*(mid->m_y - nxt->m_y));
    //qDebug() << "a = " << a << ",b = " << b << ",c = " << c;
    return (b*b + c*c - a*a) / (2 * b * c);
}

Gen3DMapPoints::~Gen3DMapPoints()
{
    clearArr();
    delPointList(m_contourPointList);
}
void Gen3DMapPoints::getDoubleBazierControls(QList<Point*>  & path,QList<Controls >  & controls,float factor){

    if(path.size() <= 1) {
        return;
    }
    if(path.size() == 2){
        if(path[0] == path[1]) return;
        Controls con1;
        con1.m_c1 = new Point(path[0]->m_x,path[0]->m_y);
        con1.m_c2 = new Point(path[1]->m_x,path[1]->m_y);
        Controls con2;
        con2.m_c1 = new Point(path[0]->m_x,path[0]->m_y);
        con2.m_c2 = new Point(path[1]->m_x,path[1]->m_y);
        controls << con1 << con2;
        return;
    }
    controls.clear();
    bool headTailLap = path[0] == path[path.length() - 1];
    for(int j = 0;j< path.length();++j){
        Point * po = path[j];
        Point * c1 = NULL;
        Point * c2 = NULL;
        Point * po_prev = NULL;
        Point * po_next = NULL;
        Point * po_next_next = NULL;
        if(j == 0){
            po_next = path[1];
            po_next_next = path[2];
            if(headTailLap){
                po_prev = path[path.length() - 2];
            }
            else{
                po_prev = path[0];
            }
        }
        else if(j == path.length() - 2){
            po_prev = path[j-1];
            po_next = path[j+1];
            if(headTailLap){
                po_next_next = path[1];
            }
            else{
                po_next_next = po_next;
            }
        }
        else if(j == path.length() - 1){
            po_prev = path[j-1];
            if(headTailLap){
                po_next = path[1];
                po_next_next = path[2];
            }
            else{
                po_next = po;
                po_next_next = po;
            }
        }
        else{
            po_prev = path[j-1];
            po_next = path[j+1];
            po_next_next = path[j+2];
        }
        c1 = new Point(po->m_x + (po_next->m_x - po_prev->m_x) * factor ,
                       po->m_y + (po_next->m_y - po_prev->m_y) * factor);
        c2 = new Point(po_next->m_x - (po_next_next->m_x - po->m_x) * factor,
                       po_next->m_y - (po_next_next->m_y - po->m_y) * factor);
        Controls con;
        con.m_c1 = c1;
        con.m_c2 = c2;
        controls.push_back(con);
    }
}
void Gen3DMapPoints::getDoubleBazierControls(QList<QList<Point*> > & pathList,
                       QList<QList<Controls > > & controlList,float factor){
    /* 如果路径首尾不相同, 表示没有闭合的需要 , 曲线控制点不会实现闭合的曲线效果 ,这让不需要闭合的路径的曲线效果更好 */
    for(int i = 0;i<pathList.size();++i){
        QList<Point*> & path = pathList[i];
        QList<Controls > controls;
        getDoubleBazierControls(path,controls,factor);
        controlList.push_back(controls);
    }
}
void Gen3DMapPoints::drawRiver(QList<QList<Point*> > & riverList,QImage & png,QPainter & svgPainter,QList<PathPainterPath> & landList){
    QPainter pngPainter(&png);
    static QPen pen;
    pen.setColor("aqua");
    pen.setWidth(2);
    pngPainter.setPen(pen);
    svgPainter.setPen(pen);
    if(m_singleLand){
        pngPainter.translate(m_translatex,m_translatey);
    }
    QList<QPainterPath> ppList;
    QList<QList<Controls > >  controlList;
    getDoubleBazierControls(riverList,controlList);
    for(int i = 0;i<riverList.size();++i){
        QList<Point*> & river = riverList[i];
        if(river.size() < 3) continue;
        QPainterPath pp;
        pp.moveTo(river[0]->m_x,river[0]->m_y);
        if(controlList[i].size() != river.size()){
            printf("WTF !!!\n");
            for(int i = 1;i<river.size();++i){
                pp.lineTo(river[i]->m_x,river[i]->m_y);
            }
        }
        else{
            QList<Controls> & cons = controlList[i];
            for(int j = 0;j<cons.size() - 1;++j){
                const Controls & c  = cons.at(j);
                pp.cubicTo(c.m_c1->m_x,c.m_c1->m_y,c.m_c2->m_x,c.m_c2->m_y,river[j+1]->m_x,river[j+1]->m_y);
            }
        }

        ppList << pp;
    }
    pngPainter.setClipping(true);
    QPainterPathStroker pps;
    pps.setWidth(2);
    for(int i = 0;i<ppList.size();++i){
        if(i >= riverList.size() || riverList[i].size() <= 0 || riverList[i][0] == NULL) continue;

        QPointF judge(riverList[i][0]->m_x,riverList[i][0]->m_y);
        QPainterPath clipPath;
        bool has = false;
        for(int j = 0;j< landList.size();++j){
            if(landList[j].m_pp.contains(judge)){
                has = true;
                clipPath = landList[j].m_pp;
                break;
            }
        }
        if(has){
            pngPainter.setClipPath(clipPath);
            pngPainter.drawPath(ppList[i]);

            QPainterPath svgpp = ppList[i];
            svgpp = pps.createStroke(svgpp);
            svgpp = svgpp.intersected(clipPath);
            svgPainter.drawPath(svgpp);
        }

    }
    delDoubleBasierControlList(controlList);
}
void Gen3DMapPoints::drawTemperature(QImage & png,QPainter & svgPainter,int shrinkScale){
    QPainter pngPainter(&png);
    if(m_singleLand){
        svgPainter.translate(-1*m_translatex,-1*m_translatey);
    }
    int col = m_xPoint - 1;
    int row = m_yPoint - 1;
    float unit = m_unit * 1.0  / shrinkScale;
    Point p_freezing1(col / 4 * unit,0);
    Point p_freezing2(col / 2 * unit,row / 6 * unit);
    Point p_freezing3(col * 0.75 * unit,0);
    QList<Point*> freezingList;
    freezingList << &p_freezing1 << &p_freezing2 << &p_freezing3;
    QList<Controls> conList;
    getDoubleBazierControls(freezingList,conList,0.2);
    QPainterPath pp_freez,pp_cold,pp_warm,pp_hot;
    pp_freez.moveTo(p_freezing1.m_x,p_freezing1.m_y);
    for(int i = 0;i< 2;++i){
        Controls & con = conList[i];
        pp_freez.cubicTo(con.m_c1->m_x,con.m_c1->m_y,con.m_c2->m_x,con.m_c2->m_y,freezingList[i+1]->m_x,freezingList[i+1]->m_y);
    }

    Point p_cold1(0,0);
    Point p_cold2(col / 2 * unit,row * 0.33 * unit);
    Point p_cold3(col * unit,0);
    QList<Point*> coldList;
    coldList << &p_cold1 << &p_cold2 << &p_cold3;
    conList.clear();
    getDoubleBazierControls(coldList,conList,0.2);
    pp_cold.moveTo(p_cold1.m_x,p_cold1.m_y);
    for(int i = 0;i<2;++i){
        Controls & con = conList[i];
        pp_cold.cubicTo(con.m_c1->m_x,con.m_c1->m_y,con.m_c2->m_x,con.m_c2->m_y,coldList[i+1]->m_x,coldList[i+1]->m_y);
    }

    Point p_warm1(0,row * 0.5 * unit);
    Point p_warm2(col * 0.5 * unit,row * 0.66 * unit);
    Point p_warm3(col* unit,row * 0.5 * unit);
    QList<Point*> warmList;
    warmList << &p_warm1 << &p_warm2 << &p_warm3;
    conList.clear();
    getDoubleBazierControls(warmList,conList,0.2);
    pp_warm.moveTo(p_warm1.m_x,p_warm1.m_y);
    for(int i = 0;i<2;++i){
        Controls & con = conList[i];
        pp_warm.cubicTo(con.m_c1->m_x,con.m_c1->m_y,con.m_c2->m_x,con.m_c2->m_y,warmList[i+1]->m_x,warmList[i+1]->m_y);
    }

    pp_hot.moveTo(0,0.99* row * unit);
    pp_hot.lineTo(col*unit,0.99*row*unit);

    QPen pen(QColor("black"));
    pen.setWidth(5);
    pen.setStyle(Qt::CustomDashLine);
    QVector<qreal> pattern;
    qreal space = 20;
    pattern << 5 << space << 20 << space;
    pen.setDashPattern(pattern);
    pngPainter.setPen(pen);
    svgPainter.setPen(pen);

    pngPainter.drawPath(pp_freez);
    pngPainter.drawPath(pp_cold);
    pngPainter.drawPath(pp_warm);
    pngPainter.drawPath(pp_hot);

    svgPainter.drawPath(pp_freez);
    svgPainter.drawPath(pp_cold);
    svgPainter.drawPath(pp_warm);
    svgPainter.drawPath(pp_hot);


    static const QString txt_freez = QString::fromLocal8Bit("严寒");
    static const QString txt_cold = QString::fromLocal8Bit("温寒");
    static const QString txt_warm = QString::fromLocal8Bit("温热");
    static const QString txt_hot = QString::fromLocal8Bit("炎热");

    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    svgPainter.setPen(pen);
    pngPainter.setPen(pen);

    QPointF hotTxtAnchor(col*0.5 * unit,row * unit - 40);

    QFont f;
    f.setPixelSize(50);
    pngPainter.setFont(f);
    svgPainter.setFont(f);

    pen.setColor(QColor("blue"));
    pngPainter.setPen(pen);
    svgPainter.setPen(pen);
    pngPainter.drawText(p_freezing2.m_x,p_freezing2.m_y-5,txt_freez);
    svgPainter.drawText(p_freezing2.m_x,p_freezing2.m_y-5,txt_freez);

    pen.setColor(QColor("skyblue"));
    pngPainter.setPen(pen);
    svgPainter.setPen(pen);
    pngPainter.drawText(p_cold2.m_x,p_cold2.m_y-5,txt_cold);
    svgPainter.drawText(p_cold2.m_x,p_cold2.m_y-5,txt_cold);

    pen.setColor(QColor("orange"));
    pngPainter.setPen(pen);
    svgPainter.setPen(pen);
    pngPainter.drawText(p_warm2.m_x,p_warm2.m_y-5,txt_warm);
    svgPainter.drawText(p_warm2.m_x,p_warm2.m_y-5,txt_warm);

    pen.setColor(QColor("red"));
    pngPainter.setPen(pen);
    svgPainter.setPen(pen);
    pngPainter.drawText(hotTxtAnchor,txt_hot);
    svgPainter.drawText(hotTxtAnchor,txt_hot);

    if(m_singleLand){
        svgPainter.translate(m_translatex,m_translatey);
    }
}
void Gen3DMapPoints::drawPath(QList<QList<Point*> > & pathList,QImage & png,QPainter & svgPainter){
    QPainter painter_png(&png);
    QList<QPainterPath> ppList;
    for(int i = 0;i<pathList.size();++i){
        QList<Point*> & path = pathList[i];

        QPainterPath pp;
        pp.moveTo(path[0]->m_x,path[0]->m_y);
        for(int i = 1;i<path.size();++i){
            pp.lineTo(path[i]->m_x,path[i]->m_y);
        }
        ppList << pp;
    }
    for(int i = 0;i<ppList.size();++i){
        painter_png.drawPath(ppList[i]);
        svgPainter.drawPath(ppList[i]);
    }
}
void Gen3DMapPoints::test(){
    {
        QSvgGenerator svgg;

        svgg.setFileName(g_path_mapResult + "map_" + QString::number(1000) + ".svg");
        svgg.setSize(QSize(2000,2000));
        QPainter svgPainter;
        svgPainter.begin(&svgg);
        QPainterPath clipPath;
        clipPath.moveTo(100,100);
        clipPath.lineTo(1000,100);
        clipPath.lineTo(1000,1000);
        clipPath.lineTo(100,1000);
        clipPath.lineTo(100,100);
        svgPainter.setClipping(true);

        qDebug() << "svgPainter.isclippling" << svgPainter.hasClipping();
        QPainterPath pp;
        pp.moveTo(500,500);
        pp.lineTo(1500,500);
        pp.lineTo(1000,1000);
        pp.lineTo(500,500);
        svgPainter.setClipPath(clipPath);
        svgPainter.drawPath(pp);

        svgPainter.end();
        return;
    }
    createArr();
    setAffectPolicy(13);
    int times = m_xPoint*m_yPoint / 10;
    resetSandBox();
    drum(times);//test
    getz();
    getContourPoints(m_contourPointList,m_z); //test
    QList<QList<Point*> > pathList;
    getContourPaths(pathList);
    sort(pathList);
    int shrinkScale = (m_xPoint-1) * m_unit / 4000;//保证最终的图形的宽度是4000像素
    shrink(pathList,shrinkScale);

    QImage png;
    QPainter svgPainter;
    QSvgGenerator svgg;
    QList<QPainterPath> ppLandList;
    drawSimpleLands(pathList,shrinkScale,png,svgPainter,svgg,ppLandList);

    QList<QList<Point*> > riverList;
    getRiverList(riverList);
    shrink(riverList,shrinkScale);

    drawPath(riverList,png,svgPainter);
    output(png,svgPainter,"test_");
}
void Gen3DMapPoints::getRiverList(QList<QList<Point*> > & riverList){
    int x = m_pArr.size();
    if(x <= 0) return;
    int y = m_pArr[0].size();
    if(y <= 0) return;
    QList<Point*> highPointList;
    for(int i = 0;i < x;++i){
        for(int j = 0;j < y;++j){
            Point* po = m_pArr[i][j];
            if(!po) continue;
            if(po->m_z > m_z) highPointList << po;
        }
    }
    qSort(highPointList.begin(),highPointList.end(),[](Point* & p1,Point* & p2){
        return p1->m_z > p2->m_z;
    }); // high --> low
    qDebug() << "highPointList.size = " << highPointList.size();
    QList<QList<Point*> > branchList;
    while(highPointList.size() > 0){
        Point * cur = highPointList[0];
        QList<Point*> river;
        river << cur;
        Point * nxt = cur;
        while(nxt){
            nxt = getNextRiverNode(nxt,m_z);
            if(nxt == NULL) break;
            river << nxt;
            if(nxt->m_z <= m_z) break;
            if(highPointList.count(nxt) <= 0) //之前已经取走了,说明这条河流和另一条河流交汇了
            {
                bool junctionSaved = false;
                for(int i = 0;i< riverList.size();++i){
                    QList<Point*> & river = riverList[i];
                    for(int j = 0;j< river.size(); ++j){
                        if(*(river[j]) == *nxt){
                            junctionSaved = true;
                            break;
                        }
                    }

                    if(junctionSaved) break;
                }
                for(int i = 0;i<branchList.size();++i){
                    QList<Point*> & branch = branchList[i];
                    for(int j = 0;j<branch.size();++j){
                        if(branch[j] == nxt){
                            junctionSaved = true;
                            break;//交汇点已经在支流上了
                        }
                    }
                    if(junctionSaved) break;
                }
                if(junctionSaved) {
                    if(river.size()>=4) branchList << river;
                    break;
                } //如果之前取走的这个节点保存起来了,就不要往后找了

                //如果之前取走的这个节点,虽然它很高,但是从一条很陡峭的路径通入大海,这时他的路径很短,没有被保存起来,就要继续往后找
            }
        }


        for(int i = 0;i<river.size();++i){
            highPointList.removeOne(river[i]);
        }
        if( river[river.size() - 1]->m_z < m_z) {

            riverList << river;
        }
    }
    sort(riverList);
    if(riverList[0].size() <= 5){
        while(riverList.size() > 3) riverList.removeAt(3);
    }
    else{
        int startIdx = 1;
        for(int i = 1;i<riverList.size();++i){
            if(riverList[i].size() < 5) {
                startIdx = i;
                break;
            }
        }
        while(riverList.size() > startIdx) riverList.removeAt(startIdx);
    }
    /* 把支流加入主流中 */
    printf("branchList.size = %d\n",branchList.size());
    QList<int> branchIdxList;
    for(int i = 0;i<branchList.size();++i){
        QList<Point*> & branch = branchList[i];
        if(branch.isEmpty() || branch[0] == NULL) continue;
        Point * junction = branch[branch.size() - 1];
        bool found = false;
        for(int j = 0;j< riverList.size();++j){
            QList<Point*> & stream = riverList[j];
            for(int k = 0;k< stream.size();++k){
                if(stream[k] == junction){
                    found = true;
                    break;
                }
            }
            if(found) break;
        }
        if(found){
            branchIdxList << i;
        }
    }
    for(int i = 0;i<branchIdxList.size();++i){
        int idx = branchIdxList[i];
        riverList << branchList[idx];
    }
}
Point* Gen3DMapPoints::getNextRiverNode(Point* cur,int cut){
    if(!cur) return NULL;
    if(cur->m_z <= cut) return NULL;//这个cur点就是入海口节点
    float fx = cur->m_x;
    float fy = cur->m_y;
    int idxx = qRound(fx / m_unit);
    int idxy = qRound(fy / m_unit);
    //不用考虑数组越界,因为外面还有一圈海洋
    QList<Point*> nxtList;
    const QList<QList<Point*> > & arr = m_pArr;
    for(int i = idxx-1;i<= idxx+1 ; ++i){
        for(int j = idxy - 1; j <= idxy + 1; ++j){
            Point * p = arr[i][j];
            if(!p) continue;
            if(p->m_z < cur->m_z) nxtList << p;
        }
    }
    float maxGap = 0;
    Point * res = NULL;
    for(int i = 0;i < nxtList.size();++i){
        if(cur->m_z - nxtList[i]->m_z > maxGap){
            maxGap = cur->m_z - nxtList[i]->m_z;
            res = nxtList[i];
        }
    }
    return res;
}
void Gen3DMapPoints::createRegion(){
    int times = m_xPoint * m_yPoint / 5;
    drum(times,0.05,0.2);
    getz();
    getContourPoints(m_contourPointList,m_z);
    printf("m_contourPointList.size =%d,total size =%d,percent=%f\n",m_contourPointList.size(),
           m_xPoint*m_yPoint,1.0*m_contourPointList.size()/(m_xPoint*m_yPoint));
    QList<QList<Point*> > pathList;
    getContourPathsForRegion(pathList);
    printf("after getContourPaths\n");
    sort(pathList);
    int shrinkScale = (m_xPoint-1) * m_unit / 2000;//保证最终的图形的宽度是2000像素
    shrink(pathList,shrinkScale);
    QImage png;
    QPainter svgPainter;
    QSvgGenerator svgg;
    QList<PathPainterPath> pppHillList;
    QList<PathPainterPath> pppLakeList;
    drawLands(pathList,shrinkScale,png,svgPainter,svgg,pppHillList,pppLakeList,7);
    output(png,svgPainter);

    //delPointList(m_obstacleList);
    delPointList(m_contourPointList);
}
void Gen3DMapPoints::createWorld(){
    int times = m_xPoint*m_yPoint / 10;
    resetSandBox();
    drum(times);//在网格上随机敲击times下，敲出times个凸块。
    getz();
    getContourPoints(m_contourPointList,m_z);
    printf("m_contourPointList.size =%d,total size =%d,percent=%f\n",m_contourPointList.size(),
           m_xPoint*m_yPoint,1.0*m_contourPointList.size()/(m_xPoint*m_yPoint));

    QList<QList<Point*> > pathList;
    getContourPaths(pathList);
    printf("after getContourPaths\n");
    sort(pathList);
    int shrinkScale = (m_xPoint-1) * m_unit / 4000;//保证最终的图形的宽度是4000像素
    shrink(pathList,shrinkScale);

    QImage png;
    QPainter svgPainter;
    QSvgGenerator svgg;
    QList<PathPainterPath> pppHillList;
    QList<PathPainterPath> pppLakeList;
    if(m_singleLand){
        drawLands(pathList,shrinkScale,png,svgPainter,svgg,pppHillList,pppLakeList,1);
    }
    else{
        drawLands(pathList,shrinkScale,png,svgPainter,svgg,pppHillList,pppLakeList,7);
    }
    if(m_fillColor){
        fillColorToLand(png,svgPainter,pppHillList);
    }
    printf("pppHillList.size = %d , pppLakeList.size = %d\n",pppHillList.size(),pppLakeList.size());
    static Mountain uselessMountain(100,100);//需要开启一个QGuiApplication,否则会段错误

    if(m_showRiver){
        QList<QList<Point*> > riverList;
        getRiverList(riverList);
        shrink(riverList,shrinkScale);
        printf("before drawRiver\n");
        drawRiver(riverList,png,svgPainter,pppHillList);
    }

    if(m_showTemperature){
        drawTemperature(png,svgPainter,shrinkScale);
    }
    if(m_showMountain){
        drawMountain(pppHillList, png,svgPainter,shrinkScale);
    }
    if(m_showKingdom){
        drawKingdom(pppHillList,png,svgPainter);
    }
    output(png,svgPainter);

    //delPointList(m_obstacleList);
    delPointList(m_contourPointList);
}
void Gen3DMapPoints::drawKingdom(const QList<PathPainterPath> & pppHillList,QImage & png,QPainter & svgPainter){
    QList<QRectF> rctList;
    static const int arr[7] = {7,5,3,2,1,1,1};
    static Kingdom king(32,32);
    QPainter painter(&png);
    if(m_singleLand){
        painter.translate(m_translatex,m_translatey);
    }
    for(int i = 0;i<pppHillList.size();++i){
        const QPainterPath & pp = pppHillList[i].m_pp;
        const QList<Point*> & path = pppHillList[i].m_path;
        int amount = 1;
        if(i >= 6) amount = arr[6];
        else amount = arr[i];
        getKingdomRectList(rctList,path,pp,amount,64,64);
        for(int j = 0;j<rctList.size();++j){
            QRectF & rct = rctList[j];
            Point * ob = new Point(rct.x(),rct.y());
            //m_obstacleList.push_back(ob);
            painter.drawImage(rct,king);
            svgPainter.drawImage(rct,king);
        }

    }
}
void Gen3DMapPoints::drawMountain(const QList<PathPainterPath> & pppHillList,QImage & png,QPainter & svgPainter,int shrinkScale){
    QList<QRectF> rctList;
    float w = 80;
    float h = 80;
    static Mountain mountain(w,h);
    QPainter pngPainter(&png);
    if(m_singleLand){
        pngPainter.translate(m_translatex,m_translatey);
    }
    QList<QList<Point*> > & mesh = m_pArr;
    float x,y;

    for(int i = 0;i<mesh.size();++i){
        for(int j = 0;j<mesh[i].size();++j){
            if(mesh[i][j]->m_z > m_mountainHeight){
                x = mesh[i][j]->m_x / shrinkScale;
                y = mesh[i][j]->m_y / shrinkScale;
                for(int k = 0;k<pppHillList.size();++k){
                    const PathPainterPath & ppp = pppHillList[k];
                    if(ppp.m_pp.contains(QPointF(x,y)) && ppp.m_pp.contains(QPointF(x+w,y)) &&
                            ppp.m_pp.contains(QPointF(x,y+h)) && ppp.m_pp.contains(QPointF(x+w,y+h))){
                        QRectF rct(x,y,w,h);
                        rctList.push_back(rct);
                        break;
                    }
                }

            }
        }
    }
    for(int j = 0;j<rctList.size();++j){
        QRectF & rct = rctList[j];
        pngPainter.drawImage(rct,mountain);
        svgPainter.drawImage(rct,mountain);
    }
}

void Gen3DMapPoints::saveMeshData(){
    QFile f(g_path_test + "mesh.txt");
    bool ok = f.open(QIODevice::WriteOnly | QIODevice::Truncate);
    if(ok){
        QString data = "z:" + QString::number(m_z) + "\n";
        data += "col:" + QString::number(m_xPoint) + ";row:" +
                QString::number(m_yPoint) + "\n";
        for(int i = 0;i<m_pArr.size();++i){
            QList<Point*> & col = m_pArr[i];
            for(int j = 0;j<col.size();++j){
                Point* p = col[j];
                data += p->toStr()+"\n";
            }
        }
        f.write(data.toLocal8Bit());
        f.close();
    }
}

void Gen3DMapPoints::readMeshData(){
    QFile f(g_path_test + "mesh.txt");
    bool ok = f.open(QIODevice::ReadOnly);
    if(!ok) return;
    QString data = QString::fromLocal8Bit(f.readAll());
    f.close();
    QStringList dataList = data.split("\n");
    int z = -1;
    int xPoint = -1;
    int yPoint = -1;
    int startIdx = 0;
    for(int i = 0;i<dataList.size();++i){
        QString & str = dataList[i];
        if(str.contains("z:")){
            z = str.split("z:")[1].toInt();
        }
        else if(str.contains("col:")){
            QStringList tmpList = str.split(";");
            if(tmpList.size() < 2) {
                printf("no row info,quit\n");
                return;
            }
            QString strCol = tmpList[0];
            QString strRow = tmpList[1];
            if(strCol.contains("col:")){
                xPoint = strCol.split("col:")[1].toInt();
            }
            if(strRow.contains("row:")){
                yPoint = strRow.split("row:")[1].toInt();
            }

        }
        if(-1 != z && -1 != xPoint && -1 != yPoint){
            startIdx = i;
            break;
        }
    }
    if(!(-1 != z && -1 != xPoint && -1 != yPoint)){
        return;
    }
    m_z = z;
    m_xPoint = xPoint;
    m_yPoint = yPoint;
    createArr();
    printf("startIdx = %d\n",startIdx);
    for(int i = 0;i<startIdx+1;++i){
        dataList.removeFirst();
    }
    int idxx = 0;
    int idxy = 0;
    QStringList xyz;
    int cnt = 0;
    for(int i = 0;i<dataList.size();++i){
        QString & str = dataList[i];
        if(str.length() < 5){
            continue;
        }
        idxx = i / 51;
        idxy = i % 51;
        if(idxx < 0 || idxx > m_xPoint-1 || idxy < 0 || idxy > m_yPoint - 1){
            printf("bad data,index out of reach,index:%d,idxx=%d,idxy=%d\n",i,idxx,idxy);
            continue;
        }
        xyz = str.split(",");
        if(xyz.size() != 3)continue;
        m_pArr[idxx][idxy]->m_x = xyz[0].toFloat();
        m_pArr[idxx][idxy]->m_y = xyz[1].toFloat();
        m_pArr[idxx][idxy]->m_z = xyz[2].toFloat();

        cnt++;
    }
    if(cnt != xPoint * yPoint){
        printf("cnt = %d , xpoint*yPoint = %d\n",cnt,xPoint * yPoint);
        printf("mesh data is not right , quit\n");
        return;
    }
}

void Gen3DMapPoints::analyse(){
#if 0
    getContourPoints(m_contourPointList,m_z);
    QList< QList<Point*> > pathList;
    getContourPaths(pathList);
    printf("after getContourPaths\n");
    sort(pathList);
    int shrinkScale = (m_xPoint-1) * m_unit / 4000;
    shrink(pathList,shrinkScale);
    m_fillColor = false;
    output(pathList,shrinkScale);
    //outputSingleLand(pathList,shrinkScale);
    delPointList(m_contourPointList);
#endif
}
void Gen3DMapPoints::getKingdomRectList(QList<QRectF> & kingdomList,const QList<Point*> & path,const QPainterPath & pp,int num,
                                        int iconWidth,int iconH){
    int unit = m_unit;
    float minx = m_xPoint * unit;
    float miny = m_yPoint * unit;
    float maxx = 0;
    float maxy = 0;
    for(int i = 0;i<path.size();++i){
        const Point* p = path[i];
        if(!p) continue;
        if(minx > p->m_x) minx = p->m_x;
        if(miny > p->m_y) miny = p->m_y;
        if(maxx < p->m_x) maxx = p->m_x;
        if(maxy < p->m_y) maxy = p->m_y;
    }
    int w = maxx - minx;
    int h = maxy - miny;
    if(w < 1 || h < 1)return;
    kingdomList.clear();

    QList<Point> meshList;//是以后诸侯国图标左上角的坐标
    int idxx = w / iconWidth;//
    int idxy = h / iconH;
    float x,y;
    for(int i = 0;i<idxx;++i){
        for(int j = 0;j<idxy;++j){
            if(i == idxx - 1){ //右边界上了
                x = minx + i*iconWidth - 0.2 * iconWidth;//减去5分之一宽度,避免图标太靠右
            }
            else if(i == 0){
                x = minx + i*iconWidth + 0.2 * iconWidth;
            }
            else{
                x = minx + i*iconWidth;
            }
            if(j == 0){
                y = miny + j*iconH + 0.2 * iconH;
            }
            else if(j == idxy - 1){
                y = miny + j*iconH - 0.2 * iconH;
            }
            else{
                y = miny + j*iconH;
            }
            meshList.push_back(Point(x,y));
        }
    }
    QList<int> delList;
    for(int i = 0;i<meshList.size();++i){
        Point & point = meshList[i];
        QPointF po(point.m_x,point.m_y);
        if(!pp.contains(po) ||
                !pp.contains(QPointF(point.m_x + iconWidth,point.m_y)) ||
                !pp.contains(QPointF(point.m_x + iconWidth,point.m_y + iconH)) ||
                !pp.contains(QPointF(point.m_x,point.m_y + iconH))){
            delList.push_front(i);
        }
    }
    for(int i = 0;i < delList.size();++i){
        meshList.removeAt(delList[i]);
    }
    if(meshList.size() <= num){
        for(int i = 0;i<meshList.size();++i){
            Point & po = meshList[i];
            QRectF rct(po.m_x,po.m_y,iconWidth,iconWidth);
            kingdomList.push_back(rct);
        }
    }
    else{
        int len = meshList.size();
        int span = len / num;
        for(int i = 0;i<num;++i){
            int idx = rand() % span + i * span;
            QRectF rct(meshList[idx].m_x,meshList[idx].m_y,iconWidth,iconWidth);
            kingdomList.push_back(rct);
        }

    }

}

void Gen3DMapPoints::getShearOffset(QList<Point*> & path,int & x,int & y,int ratio){
    x = 0;y = 0;
    int unit = m_unit;
    float minx = m_xPoint * unit;
    float miny = m_yPoint * unit;
    float maxx = 0;
    float maxy = 0;
    for(int i = 0;i<path.size();++i){
        Point* p = path[i];
        if(minx > p->m_x) minx = p->m_x;
        if(miny > p->m_y) miny = p->m_y;
        if(maxx < p->m_x) maxx = p->m_x;
        if(maxy < p->m_y) maxy = p->m_y;
    }
    x = ((m_xPoint - 1) * unit / ratio - (maxx - minx) ) / 2;
    y = ((m_yPoint - 1) * unit / ratio - (maxy - miny) ) / 2;
    x -= minx;
    y -= miny;
}

void Gen3DMapPoints::output(QImage & png,QPainter & svgPainter,QString testPrefix){
    svgPainter.end();
    png.save(g_path_mapResult + testPrefix + "map_" + QString::number(m_idxInFileName) + ".png");
}
void Gen3DMapPoints::fillColorToLand(QImage & png,QPainter & svgPainter,QList<PathPainterPath> & pppHillList){
    static const QBrush b_continent1 = QBrush(QColor("lightgray"));
    static const QBrush b_continent2 = QBrush(QColor("lightskyblue"));
    static const QBrush b_continent3 = QBrush(QColor("peachpuff"));
    static const QBrush b_continent4 = QBrush(QColor("lavender"));
    static const QBrush b_continent5 = QBrush(QColor("aliceblue"));
    static const QBrush b_continent6 = QBrush(QColor("seashell"));
    static const QBrush b_continent7 = QBrush(QColor("tan"));
    QPainter pngPainter(&png);
    if(m_singleLand) {
        pngPainter.translate(m_translatex,m_translatey);
        //svgPainter.translate(m_translatex,m_translatey);
    }
    pngPainter.setRenderHint( QPainter::Antialiasing);
    for(int i = 0;i< pppHillList.size();++i){
        const QBrush * pb = NULL;
        if( i == 0) pb = &b_continent1;
        else if(i == 1) pb = &b_continent2;
        else if(2 == i) pb = &b_continent3;
        else if(3 == i) pb = &b_continent4;
        else if(4 == i) pb = &b_continent5;
        else if(5 == i) pb = &b_continent6;
        else pb = &b_continent7;
        pngPainter.fillPath(pppHillList[i].m_pp,*pb);
        svgPainter.fillPath(pppHillList[i].m_pp,*pb);
    }
}
void Gen3DMapPoints::drawSimpleLands(QList<QList<Point*> > & pathList,int shrinkScale,QImage & png,QPainter & svgPainter,
            QSvgGenerator & svgg,QList<QPainterPath> & ppList){

    if(pathList.size() <= 0) return;
    QImage image(QSize((m_xPoint - 1) * m_unit / shrinkScale,(m_yPoint - 1) * m_unit / shrinkScale),
                 QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter_png(&image);

    painter_png.setRenderHint( QPainter::Antialiasing);

    svgg.setFileName(g_path_mapResult + "test_map_" + QString::number(m_idxInFileName) + ".svg");
    svgg.setSize(QSize((m_xPoint - 1) * m_unit / shrinkScale,(m_yPoint - 1) * m_unit / shrinkScale));

    svgPainter.begin(&svgg);
    QPen pen;
    pen.setWidth(2);
    painter_png.setPen(pen);
    svgPainter.setPen(pen);

    for(int i = 0;i<pathList.size();++i){
        QList<Point*> & path = pathList[i];

        QPainterPath pp;
        pp.moveTo(path[0]->m_x,path[0]->m_y);
        for(int i = 1;i<path.size();++i){
            pp.lineTo(path[i]->m_x,path[i]->m_y);
        }
        ppList << pp;
    }
    /* 绘制陆地轮廓线 */
    for(int i = 0;i<ppList.size();++i){
        painter_png.drawPath(ppList[i]);
        svgPainter.drawPath(ppList[i]);
    }
    png = image;
}
void Gen3DMapPoints::drawLands(QList<QList<Point*> > & pathList,int shrinkScale,QImage & png,QPainter & svgPainter,
       QSvgGenerator & svgg, QList<PathPainterPath> & pppHillList,QList<PathPainterPath> & pppLakeList,int landAmount){
    if(landAmount <= 0) landAmount = 1;
    if(pathList.size() <= 0) return;
    QImage image(QSize((m_xPoint - 1) * m_unit / shrinkScale,(m_yPoint - 1) * m_unit / shrinkScale),
                 QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter pngPainter(&image);

    pngPainter.setRenderHint( QPainter::Antialiasing);
    svgPainter.setRenderHint(QPainter::Antialiasing);
    svgg.setFileName(g_path_mapResult + "map_" + QString::number(m_idxInFileName) + ".svg");
    svgg.setSize(QSize((m_xPoint - 1) * m_unit / shrinkScale,(m_yPoint - 1) * m_unit / shrinkScale));

    svgPainter.begin(&svgg);
    QPen pen;
    pen.setWidth(2);
    pngPainter.setPen(pen);
    svgPainter.setPen(pen);
    QList<QList<Point*> > lakeList;
    //QList<QList<Point*> > hillList;
    QList<QPainterPath> ppLakeList;
    QList<QPainterPath> ppHillList;

    if(landAmount == 1){
        getShearOffset(pathList[0],m_translatex,m_translatey,shrinkScale);
        pngPainter.translate(m_translatex,m_translatey);//如果图形偏左，说明x小了，要把它向右移动，坐标系就要向左移动，offsetx>0
        svgPainter.translate(m_translatex,m_translatey);
    }
    for(int i = 0;i<pathList.size();++i){
        QList<Point*> & path = pathList[i];
        if(path.size() < 5){ //this branch should never run
            continue;
        }
        QPainterPath pp;
        getFilletPath(path,pp);

        if(isHill(path,pp,m_unit * 1.0 / shrinkScale)){
            if(ppHillList.size() >= landAmount) continue; //只保留七个大陆
            if(path.length() <= 5) continue; //太小的岛屿不要了
            ppHillList.push_back(pp);
            PathPainterPath ppp;
            ppp.m_pp = pp;
            ppp.m_path = path;
            pppHillList.push_back(ppp);
        }
        else{ //所有的湖泊都是需要的
            lakeList.push_back(path);
            ppLakeList.push_back(pp);
            PathPainterPath ppp;
            ppp.m_pp = pp;
            ppp.m_path = path;
            pppLakeList.push_back(ppp);
        }
    }
    /* 把湖泊嵌入陆地中 */
    for(int i = 0;i<ppHillList.size();++i){
        QPainterPath & ppHill = ppHillList[i];
        for(int j = 0;j<lakeList.size();++j){ //usually , this list size < 10
            QList<Point*> & lakePath = lakeList[j];
            if(ppHill.contains(QPointF(lakePath[0]->m_x,lakePath[0]->m_y))){
                ppHill.addPath(ppLakeList[j]);
                pppHillList[i].m_pp.addPath(ppLakeList[j]);
            }
        }
    }
    /* 绘制陆地轮廓线 */
    for(int i = 0;i<ppHillList.size();++i){
        pngPainter.drawPath(ppHillList[i]);
        svgPainter.drawPath(ppHillList[i]);
    }


    /* 绘制湖泊轮廓线 */
    for(int i = 0;i<ppLakeList.size();++i){
        QPainterPath & pp = ppLakeList[i];
        bool insideLand = false;
        for(int j = 0;j<ppHillList.size();++j){
            if(ppHillList[j].contains(pp.pointAtPercent(0.1))){
                insideLand = true;
                break;
            }
        }
        if(!insideLand)  //不在七大陆中的湖泊就不要了
        {
            for(int j = 0;j< pppLakeList.size();++j){
                if(pppLakeList[j].m_pp == pp){
                    pppLakeList.removeAt(j);
                    break;
                }
            }
            continue;
        }
        pngPainter.drawPath(pp);
        svgPainter.drawPath(pp);
    }
    png = image;
}


bool Gen3DMapPoints::isLake(const QList<Point*> & path,const QPainterPath & pp,float gapUnit){
    return !isHill(path,pp,gapUnit);
}
bool Gen3DMapPoints::isHill(const QList<Point*> & path,const QPainterPath & pp,float gapUnit){
    if(path.length() < 4) return true;
    float cut_z = path.at(0)->m_z;
    QPointF judge;
    bool found = false;
    for(int i = 0;i<path.size()-1;++i){
        const Point * po = path.at(i);
        if(onVerticalBeam(po,gapUnit)){
            judge = QPointF(po->m_x,floorf(po->m_y / gapUnit) * gapUnit);
            found = true;
            break;
        }
        else if(fmod(po->m_y,gapUnit) < 1 ){//on horizontal beam
            judge = QPointF(floorf(po->m_x / gapUnit) * gapUnit,po->m_y);
            found = true;
            break;
        }
        else continue;//this point was cutObtuse , let's suppose they cann't be all cutObtused in one path
    }
    if(found){
        int idx_x = qRound(judge.rx() / gapUnit);
        int idx_y = qRound(judge.ry() / gapUnit);
        if(idx_x < 0 || idx_x >= m_xPoint || idx_y < 0 || idx_y >= m_yPoint) {
            printf("Error: index out of reach,in Gen3DMapPoints::isHill\n");
            return true;
        }
        float z = m_pArr[idx_x][idx_y]->m_z;
        if(pp.contains(judge)){
            if(z > cut_z) {
                return true;
            }
            else return false;
        }
        else{
            if(z > cut_z) {
                return false;
            }
            else return true;
        }
    }
    else printf("Error: did not find judge point in Gen3DMapPoints::isHill\n");
    return true;
}
QString Gen3DMapPoints::generateNewWorld(long long idx ){
    m_idxInFileName = idx;
    m_coastDir = CoastFrame;
    createWorld();
    QString ret = g_path_mapResult + "map_" + QString::number(idx) + ".svg";
    return ret;
}
QString Gen3DMapPoints::generateNewRegion(long long idx){
    m_idxInFileName = idx;
    m_un = -1;
    setAffectPolicy(10);
    createTiltArr();
    createRegion();
    QString str = g_path_mapResult + "map_" + QString::number(idx) + ".svg";
    return str;
}
Gen3DMapPoints::Gen3DMapPoints(bool){
    m_un = 1;
    m_fillColor = true;
    m_showKingdom = false;
    m_unit = 10000;
    m_xPoint = 101;
    m_yPoint = 51;
}
Gen3DMapPoints::Gen3DMapPoints(int xPoints,int yPoints){
    m_un = -1;
    if(xPoints <= 1 || yPoints <= 1) {
        m_xPoint = 100 + 1;
        m_yPoint = 50 + 1;
    }
    else{
        m_xPoint = xPoints;
        m_yPoint = yPoints;
    }
    m_showKingdom = false;
    m_showMountain = false;
    m_showRiver = true;
    m_showTemperature = false;
    m_fillColor = true;
    m_unit = 10000;

}
Gen3DMapPoints::Gen3DMapPoints()
{
    m_un = 1;
    m_showKingdom = false;
    m_showMountain = false;
    m_showRiver = false;
    m_showTemperature = false;
    m_fillColor = true;
    m_singleLand = false;
    m_unit = 10000;
    m_xPoint = 100 + 1;//81 points,80 edges
    m_yPoint = 50 + 1;//41个点，40条边
    m_coastDir = CoastFrame;//it means world map
    /*
        这个网格的总数对后面自动计算的推荐标高是有影响的,当网格总数(m_xPoint * m_yPoint)越大,
        推荐的标高切出的陆地面积占比越小.因为网格数量大了,鼓锤的次数就多了,最大最小标高之间的值
        就大了,虽然平均标高也变大,但可能出于正态分布或其他数学原理,这个平均标高是偏小的.
    */
    createArr();
    setAffectPolicy(13);
    /*
        10000 * 0.5 = (minF) = 5000 , 5000 / 2^(13-1) = 1,
        由此可见,这个10000的单位和13的影响波层数是对应的
    */
}

void Gen3DMapPoints::getPointsInSunShape(QList<Point*> & resList,const Point* center,const QList<Point*> & srcList){
    if(center == NULL) return;
    resList.clear();
    int gap = m_unit;
    int x_min  = 0;
    int x_max  = 0;
    int y_min  = 0;
    int y_max  = 0;
    if(fmod(center->m_x,gap) < 1){ // "日" is laying down
        x_min = center->m_x - gap;
        x_max = center->m_x + gap;

        y_min = floorf(center->m_y / gap) * gap;
        y_max = y_min + gap;
        for(int i = 0;i< srcList.size();++i){
            Point * p = srcList[i];
            if(p == NULL) continue;
            if(p == center) continue;
            if((fabs(p->m_x - x_min) < 1 || fabs(p->m_x - x_max) < 1) && (p->m_y > y_min && p->m_y < y_max) ){
                //left pillar or right pillar
                resList << p;
            }
            else if((fabs(p->m_y - y_min) < 1 || fabs(p->m_y - y_max) < 1) && (p->m_x > x_min && p->m_x < x_max) ){
                //2 upside bars or 2 downside bars
                resList << p;
            }
        }
    }
    else{ // "日"
        x_min = floorf(center->m_x / gap) * gap;
        x_max = x_min + gap;
        y_min = center->m_y - gap;
        y_max = center->m_y + gap;
        for(int i = 0;i<srcList.size();++i){
            Point * p = srcList[i];
            if(p == NULL) continue;
            if(p == center) continue;
            if((fabs(p->m_y - y_min) < 1 || fabs(p->m_y - y_max) < 1) && (p->m_x < x_max && p->m_x > x_min)){
                //upside bar or downside bar
                resList << p;
            }
            else if((fabs(p->m_x - x_min) < 1 || fabs(p->m_x - x_max) < 1) && (p->m_y > y_min && p->m_y < y_max) ){
                //2 left pillars or 2 right pillars
                resList << p;
            }
        }
    }
}

void Gen3DMapPoints::shrink(QList<QList<Point*> > & pathList , int scale){
    set<Point*> visit;
    for(int i = 0;i<pathList.size();++i){
        QList<Point*> & path = pathList[i];

        for(int j = 0;j< path.length(); ++j){
            Point * p = path[j];
            if(visit.count(p) > 0) continue;
            if(p){
                p->m_x /= scale;
                p->m_y /= scale;
                visit.insert(p);
            }
        }
    }
}

bool Gen3DMapPoints::consistDiamond(Point* p,QList<Point*> & path){
    path.clear();
    if(onVerticalBeam(p,m_unit)){
        int c1y = p->m_y;
        c1y = c1y / m_unit * m_unit;
        int c2y = c1y + m_unit;
        float cx = p->m_x;
        Point * c1_left = NULL;
        Point * c1_right = NULL;
        Point * c1_down = NULL;
        Point * c2_left = NULL;
        Point * c2_right = NULL;
        Point * c2_up = NULL;
        for(int i = 0;i<m_contourPointList.size();++i){
            Point* po = m_contourPointList[i];
            if(fabs(po->m_y - c1y) < 1){
                if(po->m_x > cx - m_unit && po->m_x < cx){
                    c1_left = po;
                    continue;
                }
                if(po->m_x > cx && po->m_x < cx + m_unit){
                    c1_right = po;
                    continue;
                }
            }
            else if(fabs(po->m_y - c2y) < 1){
                if(po->m_x > cx - m_unit && po->m_x < cx){
                    c2_left = po;
                    continue;
                }
                if(po->m_x > cx && po->m_x < cx + m_unit){
                    c2_right = po;
                    continue;
                }
            }
            else if(fabs(po->m_x - cx) < 1){
                if(po->m_y > c1y - m_unit && po->m_y < c1y){
                    c1_down = po;
                    continue;
                }
                if(po->m_y > c2y && po->m_y < c2y + m_unit){
                    c2_up = po;
                    continue;
                }
            }
            if(c1_left && c1_right && c1_down) break;
            if(c2_left && c2_right && c2_up) break;
        }
        if(c1_left && c1_right && c1_down) {
            path << p << c1_left << c1_down << c1_right;
            return true;
        }
        if(c2_left && c2_right && c2_up) {
            path << p << c2_left << c2_up << c2_right;
            return true;
        }
    }
    else{
        int c1x = p->m_x;
        c1x = (c1x / m_unit) * m_unit;
        int c2x = c1x + m_unit;
        float cy = p->m_y;
        Point* c1_up = NULL;
        Point* c1_left = NULL;
        Point* c1_down = NULL;
        Point* c2_up = NULL;
        Point* c2_right = NULL;
        Point* c2_down = NULL;
        for(int i = 0;i<m_contourPointList.size();++i){
            Point* po = m_contourPointList[i];
            if(fabs(po->m_x - c1x) < 1.0){
                if(po->m_y > cy && po->m_y < cy + m_unit){
                    c1_up = po;
                    continue;
                }
                if(po->m_y < cy && po->m_y > cy - m_unit){
                    c1_down = po;
                    continue;
                }
            }
            else if(fabs(po->m_y - cy) < 1.0){
                if(po->m_x > c1x - m_unit && po->m_x < c1x){
                    c1_left = po;
                    continue;
                }
                if(po->m_x > c2x && po->m_x < c2x + m_unit){
                    c2_right = po;
                    continue;
                }
            }
            else if(fabs(po->m_x - c2x) < 1.0){
                if(po->m_y > cy && po->m_y < cy + m_unit){
                    c2_up = po;
                    continue;
                }
                if(po->m_y < cy && po->m_y > cy - m_unit){
                    c2_down = po;
                    continue;
                }
            }
            if(c1_up && c1_down && c1_left) break;
            if(c2_up && c2_down && c2_right) break;
        }
        if(c1_up && c1_down && c1_left){
            path << p << c1_up << c1_left << c1_down;
            return true;
        }
        if(c2_up && c2_down && c2_right){
            path << p << c2_up << c2_right << c2_down;
            return true;
        }
    }
    return false;
}
void Gen3DMapPoints::findAllDiamonds(QList< QList<Point*> > & pathList,set<Point*> & visit){
    //set<Point*> myVisit;
    Point* cur = NULL;
    QList<Point*> path;
    QList<Point*> srcList = m_contourPointList;
    while(srcList.size() > 0){
        cur = srcList[0];
        if(cur == NULL) break;
        srcList.removeFirst();
        bool isPartOfDiamond = consistDiamond(cur,path);
        if(isPartOfDiamond){
            //qDebug() << "path.size = " << path.size();
            bool alreadySaved = false;
            for(int i = 0;i<path.size();++i){
                if(visit.count(path[i]) > 0){
                     alreadySaved = true;
                     break;
                }
            }
            if(!alreadySaved){
                pathList << path;
                for(int i = 0;i<path.size();++i) {
                    srcList.removeOne(path[i]);
                    visit.insert(path[i]);
                }
            }
        }
    }
}
Point* Gen3DMapPoints::getContourPoint(const Point* node1,const Point* node2){
    if(!node1 || !node2) return NULL;
    if(node1 == node2) return NULL;
    if(*node1 == *node2) return NULL;
    if(fabs(node1->m_x - node2->m_x) < 1){
        int x = qRound(node1->m_x);
        float y1,y2;
        if(node1->m_y > node2->m_y){
            y1 = node2->m_y;
            y2 = node1->m_y;
        }
        else{
            y1 = node1->m_y;
            y2 = node2->m_y;
        }
        for(int i = 0;i<m_contourPointList.size();++i){
            Point* po = m_contourPointList[i];
            if(qRound(po->m_x) == x && po->m_y > y1 && po->m_y < y2){
                return po;
            }
        }
    }
    else if(fabs(node1->m_y - node2->m_y) < 1){
        int y = qRound(node1->m_y);
        float x1,x2;
        if(node1->m_x < node2->m_x){
            x1 = node1->m_x;
            x2 = node2->m_x;
        }
        else{
            x1 = node2->m_x;
            x2 = node1->m_x;
        }
        for(int i = 0;i<m_contourPointList.size();++i){
            Point* po = m_contourPointList[i];
            if(qRound(po->m_y) == y && po->m_x > x1 && po->m_x < x2){
                return po;
            }
        }
    }
    return NULL;
}
Point* Gen3DMapPoints::nextFrameNode(Point* cut){
    if(!cut) return NULL;
    int unit = m_unit;
    if(fmod(cut->m_x,unit) < 1){
        int x = qRound(cut->m_x) / unit;
        int y1 = qFloor(cut->m_y) / unit;
        int y2 = y1 + 1;
        if(x < 0 || y1 < 0 || x >= m_xPoint || y2 >= m_yPoint) return NULL;
        if(m_pArr[x][y1]->m_z > m_z) return m_pArr[x][y1];
        else return m_pArr[x][y2];
    }
    else if(fmod(cut->m_y,unit) < 1){
        int y = qRound(cut->m_y) / unit;
        int x1 = qFloor(cut->m_x) / unit;
        int x2 = x1 + 1;
        if(y < 0 || x1 < 0 || y >= m_yPoint || x2 >= m_xPoint) return NULL;
        if(m_pArr[x1][y]->m_z > m_z) return m_pArr[x1][y];
        else return m_pArr[x2][y];

    }
    return NULL;
}

Point* Gen3DMapPoints::nextFrameNode(Point* cur,QList<Point*> & rawFrame,std::set<Point*> visit){
    rawFrame.removeOne(cur);
    if(rawFrame.size() <= 0) return NULL;
    Point* nxt = rawFrame[0];
    if(nxt && nxt->m_z > m_z){
        return nxt;
    }
    else{
        Point * cut = getContourPoint(cur,nxt);\
        if(!cut){
            printf("Error! cut point is NULL\n");
        }
        return cut;
    }
}
QList<Point*> Gen3DMapPoints::nextFramePath(Point* cur,const QList<Point*>& srcList){
    std::set<Point*> visit;
    QList<Point*> path;
    path.push_back(cur);
    visit.insert(cur);
    Point * next = NULL;
    Point * prv_for6Scenario = NULL;
    QList<Point*> nextList;
    while(1){
        getPointsInSunShape(nextList,cur,srcList);
        if(nextList.size() == 1){
            path.push_back(nextList[0]);
            visit.insert(nextList[0]);
            if(path.size() > 2) break;
        }
        else if(nextList.size() == 2){
            if(path.count(nextList[0]) <= 0) next = nextList[0];
            else if(path.count(nextList[1]) <= 0) next = nextList[1];
            else{
                break;
            }
            if(next){
                path.push_back(next);
                cur = next;
                visit.insert(next);
            }
            else{
                break;
            }
        }
        else if(nextList.size() == 3){
            //不要选择对面的节点,两侧的都可以,这种情况发生的时候一定是在边界上,非闭合路径的第一个或者最后一个节点
            if(path.size() > 2) break;//退出,不需要找后面的节点了
            if(fmod(cur->m_x,m_unit) < 1){
                //axis y
                for(int i = 0;i< 3;++i){
                    if(qRound(fabs(nextList[i]->m_x - cur->m_x) ) != m_unit){
                        cur = nextList[i];
                        break;
                    }
                }
            }
            else{
                //axis x
                for(int i = 0;i<3;++i){
                    if(qRound(fabs(nextList[i]->m_y - cur->m_y) ) != m_unit){
                        cur = nextList[i];
                        break;
                    }
                }
            }
            if(path.count(cur) <= 0) {
                path << cur;
                visit.insert(cur);
            }
            else {
                printf("Error! path.count(cur) > 0\n");
                break;
            }
        }
        else if(nextList.size() == 4){
            prv_for6Scenario = cur;
            next = findNextPointForCenterWith4Points(nextList,visit,cur);
            if(next){
                path.push_back(next);
                cur = next;
                visit.insert(next);
            }
            else{
                break;
            }
        }
        else if(nextList.size() == 6){
            printf("in the most complicated situation: Scenario6\n");
            next = findNextPointForCenterWith6Points(nextList,visit,cur,prv_for6Scenario);
            if(next){
                path.push_back(next);
                cur = next;
                visit.insert(next);
                prv_for6Scenario = next;
                //this code is efficient when the next point
                //has 6 points in its sun-shape region
            }
            else{
                break;
            }
        }
    }
    return path;
}
void Gen3DMapPoints::print(const QList<Point*> & path){
    QString data;
    for(int i = 0;i < path.size();++i){
        const Point* p = path.at(i);
        if(!p) continue;
        data += "Point:" + p->str() + " ; ";
    }
    printf("path:  %s\n",data.toStdString().c_str());
}
void Gen3DMapPoints::getContourPathsForRegion(QList< QList<Point*> > & pathList){
    if(CoastNone == m_coastDir){
        getContourPaths(pathList);
        QList<Point*> frame;
        for(int i = 0;i<m_xPoint;++i)frame << m_pArr[i][0];
        for(int i = 1;i<m_yPoint;++i)frame << m_pArr[m_xPoint - 1][i];
        for(int i = m_xPoint - 2;i>=0;--i) frame << m_pArr[i][m_yPoint - 1];
        for(int i = m_yPoint - 2;i>=0;--i) frame << m_pArr[0][i];
        pathList << frame;
    }
    else{
        set<Point*> visit;
        //第一步:先把所有钻石形的小洞找出来,每个小洞都是一个path
        findAllDiamonds(pathList,visit);
        printf("after finding all diamonds\n");
        QList<Point*> leftContourPointList = m_contourPointList;
        for(set<Point*>::iterator it = visit.begin();it!= visit.end();++it){
            leftContourPointList.removeOne(*it);
        }
        //第二步:把边框连起来
        QList<Point*> frame;
        QList<Point*> rawFrame;
        for(int i = 0;i<m_xPoint;++i)rawFrame << m_pArr[i][0];
        for(int i = 1;i<m_yPoint;++i)rawFrame << m_pArr[m_xPoint - 1][i];
        for(int i = m_xPoint - 2;i>=0;--i) rawFrame << m_pArr[i][m_yPoint - 1];
        for(int i = m_yPoint - 2;i>=1;--i) rawFrame << m_pArr[0][i];
        int startIdx = 0;
        for(int i = 0;i<rawFrame.size();++i){
            if(rawFrame[i]->m_z > m_z){
                startIdx = i;
                break;
            }
        }
        printf("rawFrame.size = %d\n",rawFrame.size());
        QList<Point*> lowPart = rawFrame.mid(0,startIdx);
        for(int i = 0;i<startIdx;++i) rawFrame.removeAt(0);
        rawFrame += lowPart; //make sure first node is land
        printf("after concat,rawFrame.size = %d\n",rawFrame.size());
        Point* cur = rawFrame[0];
        std::set<Point*> frameVisit;
        while(cur){
            if(frame.count(cur) > 0) break;
            frame << cur;
            cur = nextFrameNode(cur,rawFrame,frameVisit);
            if(cur == NULL) break;
            printf("cur:%s\n",cur->str().toStdString().c_str());
            if(fmod(cur->m_x,m_unit) > 1 || fmod(cur->m_y,m_unit) > 1){//this next node is a cutNode

                QList<Point*> framePath = nextFramePath(cur,leftContourPointList);
                printf("cut node,framePath.size = %d\n",framePath.size());

                frame += framePath;
                cur = nextFrameNode(framePath[framePath.size() - 1]);
                print(framePath);
                printf("cur : %s\n",cur ? cur->str().toStdString().c_str() : "NULL");
            }
            else{
                printf("cur:%s\n",cur->str().toStdString().c_str());
                //frame << cur;
            }
            printf("frame.size = %d\n",frame.size());
        }
        for(int i = 0;i< frame.size();++i){
            leftContourPointList.removeOne(frame[i]);
        }
        printf("after while, frame.size = %d\n",frame.size());
        //第三步,把湖泊找出来
    }
}
void Gen3DMapPoints::getContourPaths(QList< QList<Point*> > & pathList){
    set<Point*> visit;
    //第一步:先把所有钻石形的小洞找出来,每个小洞都是一个path
    findAllDiamonds(pathList,visit);
    printf("after finding all diamonds\n");
    QList<Point*> leftContourPointList = m_contourPointList;
    for(set<Point*>::iterator it = visit.begin();it!= visit.end();++it){
        leftContourPointList.removeOne(*it);
    }
    while(leftContourPointList.size() > 0){
        Point * cur = leftContourPointList[0];
        QList<Point*> path;
        path.push_back(cur);
        visit.insert(cur);
        Point * next = NULL;
        Point * prv_for6Scenario = NULL;
        QList<Point*> nextList;
        while(1){
            getPointsInSunShape(nextList,cur,m_contourPointList);
            if(nextList.size() == 2){
                if(visit.count(nextList[0]) <= 0) next = nextList[0];
                else if(visit.count(nextList[1]) <= 0) next = nextList[1];
                else{
                    break;
                }
                if(next){
                    path.push_back(next);
                    cur = next;
                    visit.insert(next);
                }
                else{
                    break;
                }
            }
            else if(nextList.size() == 4){
                prv_for6Scenario = cur;
                next = findNextPointForCenterWith4Points(nextList,visit,cur);
                if(next){
                    path.push_back(next);
                    cur = next;
                    visit.insert(next);
                }
                else{
                    break;
                }
            }
            else if(nextList.size() == 6){
                printf("in the most complicated situation: Scenario6\n");
                next = findNextPointForCenterWith6Points(nextList,visit,cur,prv_for6Scenario);
                if(next){
                    path.push_back(next);
                    cur = next;
                    visit.insert(next);
                    prv_for6Scenario = next;
                    //this code is efficient when the next point
                    //has 6 points in its sun-shape region
                }
                else{
                    break;
                }
            }
            else{
                printf("Error: nextList.size = %d\n",nextList.size());
                for(int i = 0;i<path.size();++i) leftContourPointList.removeOne(path[i]);
                path.clear();
                break;
            }
        }//end of while(1)
        if(!path.isEmpty()) pathList.push_back(path);
        for(int i = 0;i<path.size();++i){
            leftContourPointList.removeOne(path[i]);
        }
        path.clear();
    }//end of while


    float x1,x2,y1,y2;
    /* 把所有的路径首尾相连 */
    for(int i = 0;i<pathList.size();++i){
        QList<Point*> & tmpList = pathList[i];
        if(tmpList.isEmpty()) continue;
        x1 = tmpList[0]->m_x;
        y1 = tmpList[0]->m_y;
        x2 = tmpList[tmpList.length() - 1]->m_x;
        y2 = tmpList[tmpList.length() - 1]->m_y;
        if(sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) < sqrt(2.0) * m_unit){
            tmpList.push_back(tmpList[0]);
        }
    }
}

bool Gen3DMapPoints::onVerticalBeam(const Point* center,float unitGap){
    float res = fmod(center->m_x,unitGap);
    if(res < 1.0) return true;
    return false;
}

Point* Gen3DMapPoints::getIsolatePoint_sunStanding(const QList<Point*> & list,const Point* center){
    Point * up = NULL;
    Point * down = NULL;
    int upCnt = 0;
    int downCnt = 0;
    for(int i = 0 ;i<list.size();++i){
        if(list[i]->m_y > center->m_y){
            up = list[i];
            upCnt++;
        }
        else{
            down = list[i];
            downCnt++;
        }
    }
    if(upCnt>1) {
        return down;
    }
    return up;
}
Point* Gen3DMapPoints::getIsolatePoint_sunLaying(const QList<Point*> & list,const Point* center){
    Point * left = NULL;
    Point * right = NULL;
    int leftCnt = 0;
    int rightCnt = 0;
    for(int i = 0;i<list.size();++i){
        if(list[i]->m_x > center->m_x){
            rightCnt++;
            right = list[i];
        }
        else{
            leftCnt++;
            left = list[i];
        }
    }
    if(rightCnt>1){
        return left;
    }
    return right;
}
Point* Gen3DMapPoints::findNextPointForCenterWith6Points(const QList<Point*> & gateList,
        const set<Point*> & visit, const Point * center,const Point* entrance){
    if(!center) return NULL;
    if(!entrance) return NULL;
    bool onV = onVerticalBeam(center,m_unit);
    QList<Point*> resList;
    qDebug() << "center:" << center->str();
    qDebug() << "prv:" << entrance->str();
    for(int i = 0;i<gateList.size();++i){

        Point * p = gateList[i];
        qDebug() << "gatePoint:" << p->str();
        if(p == NULL) continue;
        if(visit.count(p) > 0) continue;
        if(onV){ // vertical pillar
            if(fabs(fabs(center->m_x - p->m_x) - m_unit) < 0.01){
                continue; // the point right facing up to center point is not the right one
            }
            else{
                //优先选择和上一个节点形成跨过中心点的路径的出口截点，否则就会出现中心点变成了V字形的底部
                if((p->m_x - center->m_x) * (entrance->m_x - center->m_x) < 0) resList.push_back(p);
            }
        }
        else{ //horizontal beam
            if(fabs(fabs(center->m_y - p->m_y) - m_unit) < 0.01){
                continue;
            }
            else {
                if((p->m_y - center->m_y) * (entrance->m_y - center->m_y) < 0) resList.push_back(p);
            }
        }
    }
    for(int i = 0;i<resList.size();++i){
        qDebug() << "res for Scenario6:" << resList[i]->str();
    }
    if(resList.size() <= 0) return NULL;
    return resList[0];
}
Point* Gen3DMapPoints::findNextPointForCenterWith4Points(const QList<Point*> & gateList,
                                                         const set<Point*> & visit, const Point * center){
    if(center == NULL) return NULL;

    bool onV = onVerticalBeam(center,m_unit);
    Point * best = NULL; //best candedate is the point isolated on one side ,
    //cause this isolated point must visit the center point to get out of this sun-shape region
    if(onV){
        best = getIsolatePoint_sunLaying(gateList,center);
    }
    else{
        best = getIsolatePoint_sunStanding(gateList,center);
    }
    if(best && visit.count(best) <= 0) {
        return best;
    }

    for(int i = 0;i<gateList.size();++i){
        Point * p = gateList[i];
        if(p == NULL) continue;
        if(visit.count(p) > 0) continue;
        if(onV){ // vertical pillar
            if(fabs(fabs(center->m_x - p->m_x) - m_unit) < 0.01){
                continue; // the point right facing up to center point is not the right one
            }
            else{
                return p;
            }
        }
        else{
            if(fabs(fabs(center->m_y - p->m_y) - m_unit) < 0.01){
                continue;
            }
            else {
                return p;
            }
        }
    }
    return NULL;
}
void Gen3DMapPoints::getContourPoints(QList<Point*> & pList,int h){
    pList.clear();
    int idx_x = m_pArr.size(); //81
    if(idx_x <= 1) return;

    int idx_y = m_pArr.at(0).size();
    if(idx_y <= 1) return; //41

    Point * p1 = NULL; //this point
    Point * p2 = NULL; //next point
    int delta = 0;
    //vertical edges
    for(int i = 0;i< idx_x ;++i){
        for(int j = 0;j<idx_y - 1;++j){
            p1 = m_pArr[i][j];
            p2 = m_pArr[i][j+1];
            if( 1.0 * (p1->m_z - h)  * (p2->m_z - h)< 0){
                Point * p = new Point;
                p->m_x = p1->m_x;
                p->m_z = h;
                delta = fabs(h - p1->m_z) / fabs(p1->m_z - p2->m_z) * fabs(p1->m_y - p2->m_y);
                if(delta < m_unit / 50) delta = m_unit / 50;
                //with this arificial adjustment,multi-contour is fallible
                p->m_y =  delta + p1->m_y;

                pList << p;
            }
        }
    }

    //horizontal edges

    for(int j = 0;j<idx_y; ++j){
        for(int i = 0;i<idx_x - 1;++i){
            p1 = m_pArr[i][j];
            p2 = m_pArr[i+1][j];
            if(1.0* (p1->m_z - h)  * (p2->m_z - h)< 0){
                Point * p = new Point;
                p->m_y = p1->m_y;
                delta = fabs(h - p1->m_z) / fabs(p1->m_z - p2->m_z) * fabs(p1->m_x - p2->m_x);
                if(delta < m_unit / 50) delta = m_unit / 50;
                p->m_x = delta + p1->m_x;
                p->m_z = h;
                pList << p;
            }
        }
    }
}

void Gen3DMapPoints::getz(){
    if(CoastFrame == m_coastDir){//world map
        int maxz = 0;
        int minz = 0xfffffff;
        for(int i = 0;i<m_xPoint;++i){
            for(int j = 0;j<m_yPoint;++j){
                if(m_pArr[i][j]->m_z > maxz) maxz = m_pArr[i][j]->m_z;
                if(m_pArr[i][j]->m_z < minz) minz = m_pArr[i][j]->m_z;
            }
        }
        m_z = minz + (maxz - minz) / 4;
        m_mountainHeight = maxz - (maxz - minz) / 4;
        //when m_xPoint*m_yPoint = about 5000 ,the amount of contour points this z cut out is about 28% of m_xPoint*m_yPoint
    }
    else{ // regional map
        int minz = 0xfffffff;
        if(m_coastDir == CoastNone){
            int z = 0;
            for(int i = 0;i<m_xPoint;++i){
                z = m_pArr[i][0]->m_z;
                if(z < minz){
                    minz = z;
                }
                z = m_pArr[i][m_yPoint - 1]->m_z;
                if(z < minz){
                    minz = z;
                }
            }
            for(int i = 0;i<m_yPoint;++i){
                z = m_pArr[0][i]->m_z;
                if(z < minz) minz = z;
                z = m_pArr[m_xPoint - 1][i]->m_z;
                if(z < minz) minz = z;
            }
            m_z = minz - 1;
        }
        else{
            int maxz = 0;
            for(int i = 0;i<m_xPoint;++i){
                for(int j = 0;j<m_yPoint;++j){
                    if(m_pArr[i][j]->m_z > maxz) maxz = m_pArr[i][j]->m_z;
                    if(m_pArr[i][j]->m_z < minz) minz = m_pArr[i][j]->m_z;
                }
            }
            m_z = minz + (maxz - minz) / 10;
        }
    }

}

void Gen3DMapPoints::clearArr(){
    for(int i = 0;i<m_pArr.size();++i){
        for(int j = 0;j<m_pArr[i].size();++j){
            if(m_pArr[i][j]){
                delete m_pArr[i][j];
            }
        }
        m_pArr[i].clear();
    }
    m_pArr.clear();
}
void Gen3DMapPoints::createTiltArr(){
    if(m_pArr.size() > 0){
        clearArr();
    }
    for(int i = 0;i<m_xPoint;++i){
        QList<Point*> l;
        for(int j = 0;j<m_yPoint;++j){
            Point * p = new Point(i*m_unit,j*m_unit,20.0 * m_unit);
            l.push_back(p);
        }
        m_pArr.push_back(l);
    }
    //minForceFactor = 0.05,maxForceFactor = 0.2;
    const float gapUnit = 0.08 * m_unit;
    if(m_coastDir == CoastNone || CoastEast == m_coastDir){
        //西高东低
        for(int i = 0;i<m_xPoint;++i){
            for(int j = 0;j< m_yPoint;++j){
                m_pArr[i][j]->m_z += (m_xPoint - 1 - i) * gapUnit;
            }
        }
    }
    else if(m_coastDir == CoastWest){
        //西低
        for(int i = 0;i<m_xPoint;++i){
            for(int j = 0;j< m_yPoint;++j){
                m_pArr[i][j]->m_z += i * gapUnit;
            }
        }
    }
    else if(m_coastDir == CoastNorth){
        for(int i = 0;i<m_xPoint;++i){
            for(int j = 0;j< m_yPoint;++j){
                m_pArr[i][j]->m_z += j * gapUnit;
            }
        }
    }
    else if(m_coastDir == CoastSouth){
        for(int i = 0;i<m_xPoint;++i){
            for(int j = 0;j< m_yPoint;++j){
                m_pArr[i][j]->m_z += (m_yPoint - 1 - i) * gapUnit;
            }
        }
    }
    else{
        float k = (m_yPoint - 1) * 1.0 / (m_xPoint - 1);
        if(m_coastDir == CoastNorthWest || CoastSouthEast == m_coastDir) k *= -1;
        float x0,y0;//(x0,y0)所在的点是标高最大的
        if(m_coastDir == CoastNorthEast){
            x0 = 0;
            y0 = m_yPoint - 1;
        }
        else if(m_coastDir == CoastNorthWest){
            x0 = m_xPoint - 1;
            y0 = m_yPoint - 1;
        }
        else if(m_coastDir == CoastSouthEast){
            x0 = 0;
            y0 = 0;
        }
        else{//CoastSouthWest
            x0 = m_xPoint - 1;
            y0 = 0;
        }
        float dis;
        float offset = 0.08 * m_unit;
        for(int i = 0;i<m_xPoint;++i){
            for(int j = 0;j< m_yPoint;++j){
                dis = fabs((-1 * k * i + j + k * x0 - y0) / sqrt(k*k + 1));
                m_pArr[i][j]->m_z -= dis * offset;
            }
        }
    }

}

void Gen3DMapPoints::createArr(){
    if(m_pArr.size() > 0){
        clearArr();
    }
    for(int i = 0;i<m_xPoint;++i){
        QList<Point*> l;
        for(int j = 0;j<m_yPoint;++j){
            Point * p = new Point(i*m_unit,j*m_unit);
            l.push_back(p);
        }
        m_pArr.push_back(l);
    }
}
void Gen3DMapPoints::resetSandBox(){
    for(int i = 0;i<m_pArr.size();++i){
        QList<Point*> & tmpList = m_pArr[i];
        for(int j = 0; j<tmpList.size(); ++j){
            Point* p = tmpList[j];
            p->m_z = 0;
        }
    }
}
void Gen3DMapPoints::drum(int times,float minForceFactor,float maxForceFactor){
    srand(time(0));
    int indexX = 0;
    int indexY = 0;
    if(maxForceFactor <= 0) maxForceFactor = 2.0;
    if(minForceFactor <= 0) minForceFactor = 0.5;
    if(minForceFactor >= maxForceFactor){
        maxForceFactor = 2;
        minForceFactor = 0.5;
    }
    int maxF = maxForceFactor * m_unit;
    int minF = minForceFactor * m_unit;
    int thisForce = 0;
    int x1 = 0;
    int x2 = m_xPoint - 1;
    int y1 = 0;
    int y2 = m_yPoint - 1;
    if(m_affectList.size()<=0){
        cout << "Err:in drum,m_affectList.size <= 0" << endl;
        return;
    }
    int margin = m_affectList.size();
    if(margin > 3) margin = 3;
    /*
        空开的边框厚度，比如：如果作用范围半径是3（含中心点）
        那么我从第四行，第四列开始的区域中找作用点，右侧和下方也是如此。这样可以保证四周一圈的标高比较低,
        但不一定是0 , 因为m_affectList.size()大概是13,也就是有外圈12层受到作用力的影响。
    */
    x1 = margin;
    x2 -= margin;
    y1 = margin;
    y2 -= margin;
    if(x1>x2 || y1>y2) return;
    int deltax = x2 - x1 + 1;
    int deltay = y2 - y1 + 1;
    for(int i = 0;i<times;i++){
        indexX = rand() % deltax + x1;
        indexY = rand() % deltay + y1;
        thisForce = rand() % (maxF - minF) + minF;
        thisForce *= m_un;
        setForce(indexX,indexY,thisForce);
    }
}
void Gen3DMapPoints::setForce(int indexX,int indexY,int centerForce){
    int cf = centerForce;
    if(m_affectList.size() <= 0)return;
    if(indexX>=m_xPoint || indexY>=m_yPoint || indexX<0 || indexY<0) return;
    int r = m_affectList.size();//作用力的作用圈数，包含了中心点，中心点单独算作一圈
    int xLeft = indexX - (r - 1);// 减去1,是因为中心点也算在圈数里面了
    int xRight = indexX + (r- 1);
    int yUp = indexY - (r - 1);
    int yDown=indexY + (r - 1);
    if(xLeft<=-1)xLeft = 0;
    if(xRight>=m_xPoint) xRight=m_xPoint-1;
    if(yUp <= -1) yUp = 0;
    if(yDown >= m_yPoint) yDown = m_yPoint-1;
    QList<set<Point*> > cirList;
    for(int i = 0;i<r;i++){
        set<Point*> setp;
        cirList.push_back(setp);
    }
    double dis = 0.0;//distance
    for(int i = xLeft;i<=xRight;++i){
        for(int j = yUp;j<=yDown;++j){
            dis = sqrt((i-indexX)*(i-indexX) + (j-indexY)*(j-indexY)) - 0.01;
            for(int k = r-1;k>=0;k--){
                if(dis <= k && dis > k-1){
                    cirList[k].insert(m_pArr[i][j]);
                    break;
                }
            }
        }
    }
    Point* p = NULL;
    for(int i = 0;i<cirList.size();++i){
        set<Point*> & s = cirList[i];
        for(set<Point*>::iterator it = s.begin();it!=s.end();++it){
            p = *it;
            p->m_z += cf * m_affectList[i];
        }
    }
}
void Gen3DMapPoints::setForce(Point* center,int centerForce){
    int x = center->m_x / m_unit;
    int y = center->m_y / m_unit;
    setForce(x,y,centerForce);
}

void Gen3DMapPoints::setAffectPolicy(QList<float> & list){
    m_affectList.clear();
    m_affectList = list;
}
void Gen3DMapPoints::setAffectPolicy(int levelAmount){
    if(levelAmount<=0 || levelAmount>=m_xPoint) levelAmount = 4;
    m_affectList.clear();
    float ratio = 1.0;
    for(int i = 0;i<levelAmount;++i){
        m_affectList.push_back(ratio);
        ratio /= 2.0;
    }
}
bool Point::operator==(const Point& that) const{
    if(that.m_x == this->m_x && that.m_y == this->m_y) return true;
    return false;
}
QString Point::toStr() const{
    QString str = QString::number(this->m_x) + "," + QString::number(this->m_y) + "," +
            QString::number(this->m_z);
    return str;
}
QString Point::str() const{
    QString str = QString::number(this->m_x) + "," + QString::number(this->m_y);
    return str;
}
Point& Point::operator=(const Point& that){
    if(&that != this){
        m_x = that.m_x;
        m_y = that.m_y;
        m_z = that.m_z;
    }
    return *this;
}
Point::Point(float x,float y,float z):m_x(x),m_y(y),m_z(z){}
Point::Point(float x,float y){
    m_x = x;
    m_y = y;
    m_z = 0;
}
Point::Point(){
    m_x = -1;
    m_y = -1;
    m_z = 0;
}
