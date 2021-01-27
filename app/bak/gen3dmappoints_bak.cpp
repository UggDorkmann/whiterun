#include "gen3dmappoints.h"
#include "ui_gen3dmappoints.h"
#include <QFile>
#include<stdlib.h>
#include <stdio.h>
#include <QDebug>
#include <ctime>
#include <math.h>
#include <QGraphicsScene>
#include <QLocalSocket>
#include "public.h"
#include <QPainter>
#include <QSvgGenerator>
#include <QMutex>
#include <QDir>
#include <QDateTime>
static long long g_generateTimes = 0;
static QMutex g_taskMutex;
static void delPointList(QList<Point*> & list){
    for(int i = 0;i<list.size();++i){
        Point* p = list[i];
        if(p){
            delete p;
        }
    }
    list.clear();
}
static void delBasierControlList(QList<QList<Controls > > & controlList ){
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
Gen3DMapPoints::~Gen3DMapPoints()
{
    for(int i = 0;i<m_pArr.size();i++){
        QList<Point*> & l = m_pArr[i];
        for(int j = 0;j<l.size();j++){
            delete l[j];
        }
    }
    m_pArr.clear();
}
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
void Gen3DMapPoints::onDrumClicked(){
    //地球表面一个点提升的时候，它最多影响半球，所以m_y/2，这里除以3，影响更小
    int times = (m_xPoint-2)*(m_yPoint-2) / 13;
    /*
        四周的边框最小是1，两头就是2，m_x-2)*(m_y-2)就是所有可提高的点的最大数量
        13 = 1+4+8,力的作用明显的是中间三层 13个点
        这个意思就是，如果力的作用是平均的，那么所有可提高的点都会被明显提高一次。
    */
    bool has6Point = false;
    do{
        resetSandBox();
        drum(times);//在网格上随机敲击times下，敲出times个凸块。
        getz();
        getContourPoints(m_contourPointList,m_z);// max_z 的四分之一
        printf("m_contourPointList.size =%d,total size =%d,percent=%f\n",m_contourPointList.size(),
               m_xPoint*m_yPoint,1.0*m_contourPointList.size()/m_xPoint*m_yPoint);
        has6Point = hasScenario6();
    }while(has6Point);

    QList< QList<Point*> > pathList;
    printf("before getContourPaths\n");
    getContourPaths(pathList);
    printf("after getContourPaths\n");
    int shrinkScale = (m_xPoint-1) * m_unit / 4000;//保证最终的图形的宽度是4000像素

    shrink(pathList,shrinkScale);
    printf("after shrink\n");
    //sort(pathList);

    QImage image(QSize((m_xPoint - 1) * m_unit / shrinkScale,(m_yPoint - 1) * m_unit / shrinkScale),
                 QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter_png(&image);
    painter_png.setPen(QColor("black"));
    painter_png.setRenderHint(QPainter::Antialiasing);

    QSvgGenerator svgg;
    svgg.setFileName(g_path_mapResult + "map_" + QString::number(g_generateTimes) + ".svg");
    svgg.setSize(QSize(80 * m_unit / shrinkScale,40 * m_unit / shrinkScale));
    QPainter painter_svg;
    painter_svg.begin(&svgg);

    for(int i = 0;i<pathList.size();++i){
        const QList<Point*> & tmpList = pathList.at(i);
        //const QList<Controls> & conList = controlList.at(i);
        if(tmpList.size() <= 0) continue;
        if(tmpList.size() < 5){ //this branch should never run
            QString log = "path.length =" + QString::number(tmpList.size()) + " : ";
            for(int j = 0;j<tmpList.size();++j){
                const Point * _p = tmpList[j];
                log += _p->toStr() + " , ";
            }
            qDebug() << log;
            continue;
        }
        QPainterPath pp;
        pp.moveTo(tmpList[0]->m_x,tmpList[0]->m_y);

        for(int j = 1;j<tmpList.size();++j){
            const Point * po = tmpList.at(j);
            pp.lineTo(QPointF(po->m_x,po->m_y));
        }

        painter_png.drawPath(pp);
        painter_svg.drawPath(pp);
        //if(isHill(tmpList,pp,m_unit / shrinkScale)){
        //    painter.fillPath(pp,QBrush(QColor("tan")));
        //}
        //else{
        //    painter.fillPath(pp,QBrush(c));
        //}
    }
    painter_svg.end();
    image.save(g_path_mapResult + "map_" + QString::number(g_generateTimes) + ".png");
    //delBasierControlList(controlList);
    delPointList(m_contourPointList);
    printf("after delPointList(m_contourPointList)\n");
}
bool Gen3DMapPoints::isLake(const QList<Point*> & path,const QPainterPath & pp,int gapUnit){
    return !isHill(path,pp,gapUnit);
}
bool Gen3DMapPoints::isHill(const QList<Point*> & path,const QPainterPath & pp,int gapUnit){
    if(path.length() < 4) return true;
    float cut_z = path.at(0)->m_z;
    QPointF judge;
    bool found = false;
    for(int i = 0;i<path.size()-1;++i){
        const Point * po1 = path.at(i);
        const Point * po2 = path.at(i+1);
        if(onVerticalBeam(po1,gapUnit) && !onVerticalBeam(po2,gapUnit)){
            judge = QPointF(po1->m_x,po2->m_y);
            found = true;
            break;
        }
        if(onVerticalBeam(po2,gapUnit) && !onVerticalBeam(po1,gapUnit)){
            judge = QPointF(po2->m_x,po1->m_y);
            found = true;
            break;
        }

    }
    if(found){
        int idx_x = qRound(judge.rx() / gapUnit);
        int idx_y = qRound(judge.ry() / gapUnit);
        if(idx_x < 0 || idx_x >= m_xPoint || idx_y < 0 || idx_y >= m_yPoint) {
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
    return true;
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
        dt = fi.birthTime();
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
QString Gen3DMapPoints::generateAMap(){
    QMutexLocker lock(&g_taskMutex);
    delFilesSecondsAgo(5);
    onDrumClicked();
    QString ret = g_path_mapResult + "map_" + QString::number(g_generateTimes) + ".svg";
    g_generateTimes++;
    return ret;
}
Gen3DMapPoints::Gen3DMapPoints()
{
    m_unit = 10000;
    initArr();
    setAffectPolicy(13);// 10000 * 0.5 = (minF) = 5000 , 5000 / 2^(13-1) = 1,
                        // 由此可见,这个10000的单位和13的影响波层数是对应的
}

void Gen3DMapPoints::getPointsInSunShape(QList<Point*> & resList,const Point* center,const QList<Point*> & srcList){
    if(center == NULL) return;
    resList.clear();
    int gap = m_unit;
    int x_min  = 0;
    int x_max  = 0;
    int y_min  = 0;
    int y_max  = 0;
    int centx = center->m_x;
    int centy = center->m_y;
    if(centx % gap < 0.5){ // "日" is laying down
        x_min = center->m_x - gap;
        x_max = center->m_x + gap;
        y_min = (centy / gap) * gap;
        y_max = y_min + gap;
        for(int i = 0;i< srcList.size();++i){
            Point * p = srcList[i];
            if(p == NULL) continue;
            if(p == center) continue;
            if((p->m_x == x_min || p->m_x == x_max) && (p->m_y > y_min && p->m_y < y_max) ){
                //left pillar or right pillar
                resList << p;
            }
            else if((p->m_y == y_min || p->m_y == y_max) && (p->m_x > x_min && p->m_x < x_max) ){
                //2 upside bars or 2 downside bars
                resList << p;
            }
        }
    }
    else if(centy % gap < 0.5){ // "日"
        x_min = (centx / gap) * gap;
        x_max = x_min + gap;
        y_min = center->m_y - gap;
        y_max = center->m_y + gap;
        for(int i = 0;i<srcList.size();++i){
            Point * p = srcList[i];
            if(p == NULL) continue;
            if(p == center) continue;
            if((p->m_y == y_min || p->m_y == y_max) && (p->m_x < x_max && p->m_x > x_min)){
                //upside bar or downside bar
                resList << p;
            }
            else if((p->m_x == x_min || p->m_x == x_max) && (p->m_y > y_min && p->m_y < y_max) ){
                //2 left pillars or 2 right pillars
                resList << p;
            }
        }
    }
    else{
        qDebug() << "something very weird happened , point:" << center->toStr();
    }
}
bool Gen3DMapPoints::hasScenario6(){
    QList<Point*> nextList;
    for(int i = 0;i<m_contourPointList.size();++i){
        Point * p = m_contourPointList[i];
        getPointsInSunShape(nextList,p,m_contourPointList);
        if(nextList.size() == 6) return true;
    }
    return false;
}
void Gen3DMapPoints::shrink(QList<QList<Point*> > & pathList , int scale){
    set<Point*> visit;
    for(int i = 0;i<pathList.size();++i){
        QList<Point*> & path = pathList[i];

        for(int j = 0;j< path.length() - 1; ++j){
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
    set<Point*> myVisit;
    Point* cur = NULL;
    QList<Point*> path;
    while(myVisit.size() < m_contourPointList.size()){
        cur = NULL;
        for(int i = 0;i<m_contourPointList.size();++i){
            Point* p = m_contourPointList[i];
            if(myVisit.count(p) <= 0) {
                cur = p;
                break;
            }
        }
        if(cur == NULL) break;
        bool isPartOfDiamond = consistDiamond(cur,path);
        if(isPartOfDiamond){
            //qDebug() << "path.size = " << path.size();
            pathList << path;
            for(int i = 0;i<path.size();++i) {
                myVisit.insert(path[i]);
                visit.insert(path[i]);
            }
        }
        else{
            //qDebug() << "not part of diamond";
            myVisit.insert(cur);
        }
    }

}
void Gen3DMapPoints::getContourPaths(QList< QList<Point*> > & pathList){
    set<Point*> visit;
    uint size = m_contourPointList.size();
    QList<Point*> nextStartList;
    QList<Point*> tmpList;
    //第一步:先把所有钻石形的小洞找出来,每个小洞都是一个path
    findAllDiamonds(pathList,visit);
    qDebug() << "after finding all diamonds";
    QList<Point*> leftContourPointList = m_contourPointList;
    for(set<Point*>::iterator it = visit.begin();it!= visit.end();++it){
        leftContourPointList.removeOne(*it);
    }
    while(visit.size() < size){
        Point* cur = NULL;
        Point * startPoint = NULL;

        for(int j = 0;j<m_contourPointList.size();++j){
            if(visit.count( m_contourPointList[j]) <= 0){
                startPoint = m_contourPointList[j];
                nextStartList.push_back(startPoint);
                getPointsInSunShape(tmpList,startPoint,m_contourPointList);
                if(tmpList.size()==2){
                    cur = startPoint;
                    break;
                }
            }
        }
        if(cur == NULL){//没有访问的点都有四个附属点
            if(nextStartList.size() > 0) {
                cur = nextStartList[0];
                nextStartList.clear();
            }
            else continue;
        }
        else nextStartList.clear();

        QList<Point*> path;
        path.push_back(cur);
        visit.insert(cur);
        Point * next = NULL;
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
            }
            else if(nextList.size() == 4){
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
                qDebug() << "in the most complicated situation: Scenario6";
                next = findNextPointForCenterWith6Points(nextList,visit,cur);
                if(next){
                    path.push_back(next);
                    cur = next;
                    visit.insert(next);
                }
                else{
                    break;
                }
            }
        }
        if(!path.isEmpty()){
            pathList.push_back(path);
            path.clear();
        }
    }//end of while
    float x1,x2,y1,y2;
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
bool Gen3DMapPoints::onVerticalBeam(const Point* center,int unitGap){
    int x = qRound(center->m_x);
    if(x % unitGap < 1) return true;
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
                                                         const set<Point*> & visit, const Point * center){
    if(!center) return NULL;
    bool onV = onVerticalBeam(center,m_unit);
    QList<Point*> resList;
    for(int i = 0;i<gateList.size();++i){
        Point * p = gateList[i];
        if(p == NULL) continue;
        if(visit.count(p) > 0) continue;
        if(onV){ // vertical pillar
            if(fabs(fabs(center->m_x - p->m_x) - m_unit) < 0.01){
                continue; // the point right facing up to center point is not the right one
            }
            else{
                resList.push_back(p);
            }
        }
        else{
            if(fabs(fabs(center->m_y - p->m_y) - m_unit) < 0.01){
                continue;
            }
            else {
                resList.push_back(p);
            }
        }
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
    int maxz = 0;
    int minz = 0xfffffff;
    for(int i = 0;i<m_xPoint;++i){
        for(int j = 0;j<m_yPoint;++j){
            if(m_pArr[i][j]->m_z > maxz) maxz = m_pArr[i][j]->m_z;
            if(m_pArr[i][j]->m_z < minz) minz = m_pArr[i][j]->m_z;
        }
    }
    m_z = minz + (maxz - minz) / 4;
    cout << "suggest height:" << m_z << endl;
}


void Gen3DMapPoints::initArr(){
    m_xPoint = 101;//81 points,80 edges
    m_yPoint = 51;//41个点，40条边

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
void Gen3DMapPoints::drum(int times,float force){
    srand(time(0));
    int indexX = 0;
    int indexY = 0;
    if(force <= 0) force = 2.0;
    int maxF = force * m_unit;
    int minF = 0.5 * m_unit;
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
bool Point::operator==(const Point& that){
    if(that.m_x == this->m_x && that.m_y == this->m_y) return true;
    return false;
}
QString Point::toStr() const{
    QString str = QString::number(this->m_x) + "," + QString::number(this->m_y) + "," +
            QString::number(this->m_z);
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
