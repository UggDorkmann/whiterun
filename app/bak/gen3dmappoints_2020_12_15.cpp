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
void Gen3DMapPoints::getLongRiverList(QList<QList<Point*> > & riverList){
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
        return p1->m_z < p2->m_z;
    }); // low --> high
    std::set<Point*> visit;

    while(highPointList.size() > 0){
        Point * cur = highPointList[0];
        QList<Point*> river;
        river << cur;

        Point * nxt = cur;
        bool isBranch = false;
        while(nxt){
            nxt = getNextUpstreamNode(nxt,visit);
            if(nxt == NULL) break;
            river << nxt;
            if(highPointList.count(nxt) <= 0) //之前已经取走了,说明这条河流和另一条河流交汇了
            {
                isBranch = true;
                break;
            }
        }
        if(river.size() >= 10) riverList << river;
        for(int i = 0;i<river.size();++i){
            highPointList.removeOne(river[i]);
            visit.insert(river[i]);
        }
    }
}
Point* Gen3DMapPoints::getNextUpstreamNode(Point* cur,std::set<Point*> & visit){
    if(!cur) return NULL;
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
            if(p->m_z > cur->m_z) nxtList << p;
        }
    }
    float minGap = 2 * m_unit;
    Point * res = NULL;
    for(int i = 0;i < nxtList.size();++i){
        if(nxtList[i]->m_z - cur->m_z  < minGap){
            minGap = nxtList[i]->m_z - cur->m_z;
            res = nxtList[i];
        }
    }
    return res;
}
void Gen3DMapPoints::addRiverDetails(Point* cur,Point * nxt,QList<Point*> & river){
    if(!cur || !nxt) return;
    static const int num = 8;
    //注释中的方向都是按照从左上角流向右下角来考虑的
    static QList<QList<int> > arrtt;
    static QList<QList<int> > arrhv;
    static bool hasInit = false;
    srand(time(NULL));
    if(!hasInit){
        QList<int> tt1 = {0,1,2};
        QList<int> tt2 = {-1,0,1,2};
        QList<int> tt3 = {-1,0,1,2};
        QList<int> tt4 = {-2,-1,0,1,2};
        QList<int> tt5 = {-2,-1,0,1,2};
        QList<int> tt6 = {-2,-1,0,1};
        QList<int> tt7 = {-2,-1,0,1};
        QList<int> tt8 = {-2,-1,0};
        arrtt << tt1 << tt2 << tt3 << tt4 << tt5 << tt6 << tt7 << tt8;
        QList<int> hv1 = {-2,-1,0,1,2};
        QList<int> hv2 = {-3,-2,-1,0,1,2,3};
        QList<int> hv3 = {-4,-3,-2,-1,0,1,2,3,4};
        QList<int> hv4 = {-5,-4,-3,-2,-1,0,1,2,3,4,5};
        QList<int> hv5 = {-5,-4,-3,-2,-1,0,1,2,3,4,5};
        QList<int> hv6 = {-4,-3,-2,-1,0,1,2,3,4};
        QList<int> hv7 = {-3,-2,-1,0,1,2,3};
        QList<int> hv8 = {-2,-1,0,1,2};
        arrhv << hv1 << hv2 << hv3 << hv4 << hv5 << hv6 << hv7 << hv8;
        hasInit = true;
    }
    QList<Point*> seg;
    int lastIdx = 0;
    QList<int> candedateList;
    if(fabs(cur->m_x - nxt->m_x) < 1){
        //vertical
        float offsety = (nxt->m_y - cur->m_y) / (num + 1);
        float offsetx = fabs(offsety);
        for(int i = 0;i<num;++i){
            Point* node = new Point(cur->m_x,cur->m_y + (i + 1) * offsety);
            seg << node;
        }
        for(int i = 0;i<num;++i){
            if(0 == i){
                int idx = rand() % (arrhv[0].size());
                lastIdx = arrhv[0][idx];
                seg[0]->m_x += offsetx * lastIdx;
            }
            else{
                candedateList.clear();
                const QList<int> & idxList = arrhv.at(i);
                for(int j = 0;j<idxList.size();++j){
                    if(idxList.at(j) >= lastIdx - 2 && idxList.at(j) <= lastIdx + 2){
                        candedateList << idxList.at(j);
                    }
                }
                int len = candedateList.size();
                if(len <= 0) continue;
                lastIdx = candedateList.at(rand() % len);
                seg[i]->m_x += offsetx * lastIdx;
            }
        }
    }
    else if(fabs(cur->m_y - nxt->m_y) < 1){
        //horizontal
        float offsetx = (nxt->m_x - cur->m_x) / (num + 1);
        float offsety = fabs(offsetx);
        for(int i = 0;i<num;++i){
            Point* node = new Point(cur->m_x + (i+1) * offsetx,cur->m_y);
            seg << node;
        }
        for(int i = 0;i<num;++i){
            if(0 == i){
                int idx = rand() % (arrhv[0].size());
                lastIdx = arrhv[0][idx];
                seg[0]->m_y += offsety * lastIdx;
            }
            else{
                candedateList.clear();
                const QList<int> & idxList = arrhv.at(i);
                for(int j = 0;j<idxList.size();++j){
                    if(idxList.at(j) >= lastIdx - 2 && idxList.at(j) <= lastIdx + 2){
                        candedateList << idxList.at(j);
                    }
                }
                int len = candedateList.size();
                if(len <= 0) continue;
                lastIdx = candedateList.at(rand() % len);
                seg[i]->m_y += offsety * lastIdx;
            }
        }
    }
    else{
        //tilt
        float offsetx = (nxt->m_x - cur->m_x) / (num + 1);
        float offsety = (nxt->m_y - cur->m_y) / (num + 1);
        for(int i = 0;i<num;++i){
            Point* node = new Point(cur->m_x + (i+1) * offsetx, cur->m_y + (i+1) * offsety);
            seg << node;
        }
        for(int i = 0;i< num; ++i){
            if(0 == i){
                int idx = rand() % (arrtt[0].size());
                lastIdx = arrtt[0][idx];
                seg[0]->m_x += offsetx * lastIdx;
            }
            else{
                candedateList.clear();
                const QList<int> & idxList = arrtt.at(i);
                for(int j = 0;j<idxList.size();++j){
                    if(idxList.at(j) >= lastIdx - 2 && idxList.at(j) <= lastIdx + 2){
                        candedateList << idxList.at(j);
                    }
                }
                int len = candedateList.size();
                if(len <= 0) {
                    continue;
                }
                lastIdx = candedateList.at(rand() % len);
                seg[i]->m_x += offsetx * lastIdx;
            }
        }
    }
    Point * term = new Point(nxt->m_x,nxt->m_y);
    seg << term;
    river += seg;
}
void Gen3DMapPoints::addRiverDetails(QList<QList<Point*> > & riverList){
    QList<QList<Point*> > newRiverList;
    Point * cur = NULL;
    Point * nxt = NULL;
    for(int i = 0;i<riverList.size();++i){
        QList<Point*> & river = riverList[i];
        if(river.size() <= 0) continue;
        QList<Point*> newRiver;
        Point* start = new Point(river[0]->m_x,river[0]->m_y);
        newRiver << start;
        for(int j = 0;j<river.size() - 1;++j){
            cur = river[j];
            nxt = river[j+1];

            addRiverDetails(cur,nxt,newRiver);
        }
        newRiverList << newRiver;
    }
    riverList = newRiverList;
}
static void getCenter(Point & center,float x1,float y1,float x2,float y2,Point * nxt){
    if(fabs(nxt->m_y - y1) < 1){// cur--nxt is horizontal
        if(fabs(nxt->m_x - x2) < 1) // nxt--far is vertical
        {
            center.m_x = x1;
            center.m_y = y2;
            return;
        }
        else{//normal
            float k = -1 / ((y2 - nxt->m_y) / (x2 - nxt->m_x));
            //y - y2 = k(x - x2);
            float y = k * (x1 - x2) + y2;
            center.m_x = x1;
            center.m_y = y;
            return;
        }
    }
    else if(fabs(nxt->m_x - x1) < 1){ // cur--nxt is vertical
        if(fabs(y2 - nxt->m_y) < 1){// nxt--far is horizontal
            center.m_x = x2;
            center.m_y = y1;
            return;
        }
        else{//normal
            float k = -1 / ((y2 - nxt->m_y) / (x2 - nxt->m_x));
            float x = (y1 - y2) / k + x2;
            center.m_x = x;
            center.m_y = y1;
            return;
        }
    }
    else{
        if(fabs(nxt->m_x - x2) < 1) // nxt--far is vertical
        {
            float k = -1 / ((y1 - nxt->m_y) / (x1 - nxt->m_x));
            float x = (y2 - y1) / k + x1;
            center.m_x = x;
            center.m_y = y2;
        }
        else if(fabs(nxt->m_y - y2) < 1){ // nxt--far is horizontal
            float k = -1 / ((y1 - nxt->m_y) / (x1 - nxt->m_x));
            float y = k*(x2 - x1) + y1;
            center.m_x = x2;
            center.m_y = y;
        }
        else{ //most scenario
            float k1 = -1 / ((y1 - nxt->m_y) / (x1 - nxt->m_x));
            float k2 = -1 / ((y2 - nxt->m_y) / (x2 - nxt->m_x));
            float x = (k1 * x1 - k2 * x2 + y2 - y1) / (k1 - k2);
            float y = y1 + k1 * (x - x1);
            center.m_x = x;
            center.m_y = y;
        }
    }
}
static float getAngle(Point & center,float arcX,float arcY){
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
static void extend(Point* cur,Point* nxt,Point* far,QPainterPath & pp){
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
    if(arcStartX < 0 || arcStartX > 100*10000 || arcStartY < 0 || arcStartY > 100*10000){
        printf("arcStartX = %f,arcStarty = %f\n",arcStartX,arcStartY);
    }

    float arcEndX = nxt->m_x + ratio2 * (far->m_x - nxt->m_x);
    float arcEndY = nxt->m_y + ratio2 * (far->m_y - nxt->m_y);

    Point center;
    getCenter(center,arcStartX,arcStartY,arcEndX,arcEndY,nxt);
    qDebug() << "center:" << center.str();

    float r = sqrt( (center.m_x - arcStartX) * (center.m_x - arcStartX) +
                    (center.m_y - arcStartY) * (center.m_y - arcStartY));

    //qDebug() <<  "r = " << r;

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
    qDebug() << "startAngle:" << startAngle << ",spanAngle = " << spanAngle;
    pp.arcTo(rct,startAngle,spanAngle);
}
static void getFilletPath(const QList<Point*> & path,QPainterPath & pp){
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
        extend(cur,nxt,far,pp);
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
static double getCos(Point* cur,Point* prv,Point* nxt){
    if(!cur || !prv || !nxt) return 0;
    double a = sqrt((prv->m_x - nxt->m_x)*(prv->m_x - nxt->m_x) + (prv->m_y - nxt->m_y)*(prv->m_y - nxt->m_y));
    double b = sqrt((prv->m_x - cur->m_x)*(prv->m_x - cur->m_x) + (prv->m_y - cur->m_y)*(prv->m_y - cur->m_y));
    double c = sqrt((cur->m_x - nxt->m_x)*(cur->m_x - nxt->m_x) + (cur->m_y - nxt->m_y)*(cur->m_y - nxt->m_y));
    //qDebug() << "a = " << a << ",b = " << b << ",c = " << c;
    return (b*b + c*c - a*a) / (2 * b * c);
}
/* 切出一个钝角 */
static void cutObtuse(Point* cur,Point* prv,Point* nxt){
    float ratio = 0.2;
    float x1 = prv->m_x - (prv->m_x - cur->m_x) * ratio;
    float y1 = prv->m_y - (prv->m_y - cur->m_y) * ratio;
    float x2 = nxt->m_x - (nxt->m_x - cur->m_x) * ratio;
    float y2 = nxt->m_y - (nxt->m_y - cur->m_y) * ratio;
    cur->m_x = (x1 + x2) / 2;
    cur->m_y = (y1 + y2) / 2;
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
    delPointList(m_contourPointList);
}

void Gen3DMapPoints::fillet(QList<Point*> & path){
    int len = path.length();
    if(len < 4) return;
    Point* prv = NULL;
    Point* nxt = NULL;
    Point* cur = NULL;
    bool headTailLap = true;
    if(path[len-1] != path[0]) headTailLap = false;
    int loopTime = len-1;
    if(!headTailLap) loopTime = len;
    for(int i = 0;i<loopTime;++i){
        cur = path[i];
        if(i == 0){
            nxt = path[1];
            if(headTailLap) prv = path[len-2];
            else prv = path[len-1];
        }
        else if(i == len-1){
            prv = path[i-1];
            if(headTailLap) nxt = path[1];
            else nxt = path[0];
        }
        else{
            prv = path[i-1];
            nxt = path[i+1];
        }
        double cosCur = getCos(cur,prv,nxt);
        if(cosCur >= 0.7071){ //cos45
            cutObtuse(cur,prv,nxt);
        }
    }
}

void Gen3DMapPoints::getDoubleBazierControls(QList<QList<Point*> > & pathList,
                       QList<QList<Controls > > & controlList){
    float factor = 0.2;
    for(int i = 0;i<pathList.size();++i){
        QList<Point*> & path = pathList[i];
        if(path.length() < 5) { //include the repeated head and tail node
            qDebug() << "path length < 5 ";
            continue;
        }
        QList<Controls > controls;
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
                    po_prev = path[path.length() - 1];
                }
            }
            else if(j == path.length() - 2){
                po_prev = path[j-1];
                po_next = path[j+1];
                if(headTailLap){
                    po_next_next = path[1];
                }
                else{
                    po_next_next = path[0];
                }
            }
            else if(j == path.length() - 1){
                po_prev = path[j-1];
                if(headTailLap){
                    po_next = path[1];
                    po_next_next = path[2];
                }
                else{
                    po_next = path[0];
                    po_next_next = path[1];
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
        controlList.push_back(controls);
    }
}
Point * Gen3DMapPoints::getPerpendicularOffsetPoint(Point * cur,Point* nxt, Point* far){
    if(!cur || !nxt || !far) return NULL;
    float len = sqrt((cur->m_x - nxt->m_x) * (cur->m_x - nxt->m_x) +
                     (cur->m_y - nxt->m_y) * (cur->m_y - nxt->m_y));
    float offset = len / 6;
    Point mid((cur->m_x + nxt->m_x)/2,(cur->m_y + nxt->m_y)/2);
    Point bak1,bak2;
    if(fabs(cur->m_x - nxt->m_x) < 1){
        bak1.m_x = cur->m_x - offset;
        bak1.m_y = mid.m_y;
        bak2.m_x = cur->m_x + offset;
        bak2.m_y = mid.m_y;
    }
    else if(fabs(cur->m_y - nxt->m_y) < 1){
        bak1.m_y = cur->m_y + offset;
        bak2.m_y = cur->m_y - offset;
        bak1.m_x = mid.m_x;
        bak2.m_x = mid.m_x;
    }
    else if(cur->m_x > nxt->m_x && cur->m_y > nxt->m_y ||
            cur->m_x < nxt->m_x && cur->m_y < nxt->m_y){
        //斜率大于0
        float leny = fabs(cur->m_y - nxt->m_y);
        float lenx = fabs(cur->m_x - nxt->m_x);
        float hypotenuse = sqrt(leny*leny + lenx*lenx);
        float ratio = offset / hypotenuse;
        leny *= ratio;
        lenx *= ratio;
        bak1.m_x = mid.m_x - leny;
        bak1.m_y = mid.m_y + lenx;
        bak2.m_x = mid.m_x + leny;
        bak2.m_y = mid.m_y - lenx;
    }
    else{
        //斜率小于0
        float leny = fabs(cur->m_y - nxt->m_y);
        float lenx = fabs(cur->m_x - nxt->m_x);
        float hypotenuse = sqrt(leny*leny + lenx*lenx);
        float ratio = offset / hypotenuse;
        leny *= ratio;
        lenx *= ratio;
        bak1.m_x = mid.m_x - leny;
        bak1.m_y = mid.m_y - lenx;
        bak2.m_x = mid.m_x + leny;
        bak2.m_y = mid.m_y + lenx;
    }
    float len1 = sqrt( (bak1.m_x - far->m_x) * (bak1.m_x - far->m_x) +
                       (bak1.m_y - far->m_y) * (bak1.m_y - far->m_y) );
    float len2 = sqrt( (bak2.m_x - far->m_x) * (bak2.m_x - far->m_x) +
                       (bak2.m_y - far->m_y) * (bak2.m_y - far->m_y) );
    if(len1 > len2){
        Point * ret = new Point(bak1.m_x,bak1.m_y);
        return ret;
    }
    else{
        Point * ret = new Point(bak2.m_x,bak2.m_y);
        return ret;
    }
}
void Gen3DMapPoints::getSingleBazierControls(QList<QList<Point*> > & pathList,
                                             QList<QList<Point*> > & controlList){
    //路径的最后一个和第一个是一样的
    Point * cur = NULL;//----point1----point2----point3---- ==> cur=point1,nxt=point2,far=point3
    Point * nxt = NULL;
    Point * far = NULL;
    for(int i = 0;i<pathList.size();++i){
        QList<Point*> & path = pathList[i];
        QList<Point*> conList;
        for(int j = 0;j<path.size() - 1;++j){
            cur = path[j];
            if(j == path.size() - 2){
                nxt = path[0];// == path[0]
                far = path[1];
            }
            else{
                nxt = path[j+1];
                far = path[j+2];
            }
            Point* con = getPerpendicularOffsetPoint(cur,nxt,far);
            if(con) conList.push_back(con);

        }
        controlList.push_back(conList);
    }
}

void Gen3DMapPoints::saveContourPath(QList< QList<Point*> > & pathList){
    QString data;
    QString str;
    for(int i = 0;i<pathList.size();++i){
        QList<Point*> & path = pathList[i];
        str = "path:";
        for(int j = 0;j<path.size();++j){
            str += path[j]->str() + ";";
        }
        data += str + "\n";
    }
    QFile f(g_path_test + "path.txt");
    bool ok = f.open(QIODevice::WriteOnly | QIODevice::Truncate);
    if(ok){
        f.write(data.toLocal8Bit());
        f.close();
    }
}
void Gen3DMapPoints::onDrumClicked(){
    int times = m_xPoint*m_yPoint / 10;
    resetSandBox();
    drum(times);//在网格上随机敲击times下，敲出times个凸块。
    getz();
    getContourPoints(m_contourPointList,m_z);
    printf("m_contourPointList.size =%d,total size =%d,percent=%f\n",m_contourPointList.size(),
           m_xPoint*m_yPoint,1.0*m_contourPointList.size()/(m_xPoint*m_yPoint));

    QList< QList<Point*> > pathList;
    getContourPaths(pathList);
    printf("after getContourPaths\n");
    sort(pathList);
    int shrinkScale = (m_xPoint-1) * m_unit / 4000;//保证最终的图形的宽度是4000像素
    shrink(pathList,shrinkScale);

    if(m_singleLand){
        outputSingleLand(pathList,shrinkScale);
    }
    else{
        output(pathList,shrinkScale);
    }
    delPointList(m_contourPointList);
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
    printf("dataList.size = %d\n",dataList.size());
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
void Gen3DMapPoints::getContourPathList(QList<QList<Point*> > & pathList){
    getContourPoints(m_contourPointList,m_z);
    getContourPaths(pathList);
    printf("after getContourPaths\n");
    sort(pathList);
    int shrinkScale = (m_xPoint-1) * m_unit / 4000;
    shrink(pathList,shrinkScale);
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
    output(pathList,shrinkScale);
    delPointList(m_contourPointList);
#endif
}
void Gen3DMapPoints::getKingdomRectList(QList<QRectF> & kingdomList,QList<Point*> & path,QPainterPath & pp){

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
    //printf("maxx = %f,maxy = %f,minx = %f,miny = %f\n",maxx,maxy,minx,miny);
    //printf("offsetx = %d,offsety = %d\n",x,y);
}
void Gen3DMapPoints::outputSingleLand(QList<QList<Point*> > & pathList,int shrinkScale){
    QImage image(QSize((m_xPoint - 1) * m_unit / shrinkScale,(m_yPoint - 1) * m_unit / shrinkScale),
                 QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter_png(&image);
    painter_png.setRenderHint( QPainter::Antialiasing);

    QSvgGenerator svgg;
    svgg.setFileName(g_path_mapResult + "map_" + QString::number(m_idxInFileName) + ".svg");
    svgg.setSize(QSize((m_xPoint - 1) * m_unit / shrinkScale,(m_yPoint - 1) * m_unit / shrinkScale));
    QPainter painter_svg;
    painter_svg.begin(&svgg);

    QList<QList<Point*> > lakeList;
    //QList<QList<Point*> > hillList;
    QList<QPainterPath> ppLakeList;
    QList<QPainterPath> ppHillList;
    int offsetx,offsety;
    getShearOffset(pathList[0],offsetx,offsety,shrinkScale);
    painter_png.translate(offsetx,offsety);//如果图形偏左，说明x小了，要把它向右移动，坐标系就要向左移动，offsetx>0
    painter_svg.translate(offsetx,offsety);
    if(m_fillColor){
        for(int i = 0;i<pathList.size();++i){
            QList<Point*> & path = pathList[i];
            if(path.size() < 5){ //this branch should never run
                continue;
            }
            QPainterPath pp;
            getFilletPath(path,pp);
            if(isHill(path,pp,m_unit * 1.0 / shrinkScale)){
                if(ppHillList.size() >= 1) continue; //只保留1个大陆
                ppHillList.push_back(pp);
            }
            else{ //所有的湖泊都是需要的
                lakeList.push_back(path);
                ppLakeList.push_back(pp);
            }

        }

        QPainterPath & ppHill = ppHillList[0];
        for(int j = 0;j<lakeList.size();++j){ //usually , this list size < 10
            QList<Point*> & lakePath = lakeList[j];
            if(ppHill.contains(QPointF(lakePath[0]->m_x,lakePath[0]->m_y))){
                ppHill.addPath(ppLakeList[j]);
            }
        }
        static const QBrush b_continent1 = QBrush(QColor("beige"));
        painter_png.drawPath(ppHillList[0]);
        painter_svg.drawPath(ppHillList[0]);
        painter_png.fillPath(ppHillList[0],b_continent1);
        painter_svg.fillPath(ppHillList[0],b_continent1);
    }
    else{
        for(int i = 0;i<pathList.size();++i){
            QList<Point*> & path = pathList[i];
            if(path.size() < 5){ //this branch should never run
                continue;
            }
            QPainterPath pp;
            getFilletPath(path,pp);
            if(isHill(path,pp,m_unit * 1.0 / shrinkScale)){
                if(ppHillList.size() >= 1) continue; //只保留七个大陆
                ppHillList.push_back(pp);
            }
            else{ //所有的湖泊都是需要的
                //lakeList.push_back(path);
                ppLakeList.push_back(pp);
            }
        }

        painter_png.drawPath(ppHillList[0]);
        painter_svg.drawPath(ppHillList[0]);
    }
    /* 两种模式(线条,着色)下,湖泊都不需要填充颜色 */
    for(int i = 0;i<ppLakeList.size();++i){
        QPainterPath & pp = ppLakeList[i];
        bool insideLand = false;
        for(int j = 0;j<ppHillList.size();++j){
            if(ppHillList[j].contains(pp.pointAtPercent(0.1))){
                insideLand = true;
                break;
            }
        }
        if(!insideLand) continue;
        painter_png.drawPath(ppLakeList[i]);
        painter_svg.drawPath(ppLakeList[i]);
    }

    painter_svg.end();
    image.save(g_path_mapResult + "map_" + QString::number(m_idxInFileName) + ".png");
}
void Gen3DMapPoints::output(QList<QList<Point*> > & pathList,int shrinkScale){
    QImage image(QSize((m_xPoint - 1) * m_unit / shrinkScale,(m_yPoint - 1) * m_unit / shrinkScale),
                 QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter_png(&image);
    painter_png.setRenderHint( QPainter::Antialiasing);

    QSvgGenerator svgg;
    svgg.setFileName(g_path_mapResult + "map_" + QString::number(m_idxInFileName) + ".svg");
    svgg.setSize(QSize((m_xPoint - 1) * m_unit / shrinkScale,(m_yPoint - 1) * m_unit / shrinkScale));
    QPainter painter_svg;
    painter_svg.begin(&svgg);

    QList<QList<Point*> > lakeList;
    //QList<QList<Point*> > hillList;
    QList<QPainterPath> ppLakeList;
    QList<QPainterPath> ppHillList;
    if(m_fillColor){
        for(int i = 0;i<pathList.size();++i){
            QList<Point*> & path = pathList[i];
            if(path.size() < 5){ //this branch should never run
                QString str = "path.length =" + QString::number(path.size()) + " : ";
                for(int j = 0;j<path.size();++j){
                    const Point * _p = path[j];
                    str += _p->toStr() + " , ";
                }
                qDebug() << str;
                continue;
            }
            QPainterPath pp;
            getFilletPath(path,pp);
            //pp.moveTo(QPointF(path[0]->m_x,path[0]->m_y));
            //for(int i = 1;i<path.size();++i){
            //    pp.lineTo(QPointF(path[i]->m_x,path[i]->m_y));
            //}

            if(isHill(path,pp,m_unit * 1.0 / shrinkScale)){
                if(ppHillList.size() >= 7) continue; //只保留七个大陆
                if(path.length() <= 5) continue; //太小的岛屿不要了
                //hillList.push_back(path);
                ppHillList.push_back(pp);
            }
            else{ //所有的湖泊都是需要的
                lakeList.push_back(path);
                ppLakeList.push_back(pp);
            }

        }
        for(int i = 0;i<ppHillList.size();++i){
            QPainterPath & ppHill = ppHillList[i];
            for(int j = 0;j<lakeList.size();++j){ //usually , this list size < 10
                QList<Point*> & lakePath = lakeList[j];
                if(ppHill.contains(QPointF(lakePath[0]->m_x,lakePath[0]->m_y))){
                    ppHill.addPath(ppLakeList[j]);
                }
            }
        }
        static const QBrush b_continent1 = QBrush(QColor("beige"));
        static const QBrush b_continent2 = QBrush(QColor("lightskyblue"));
        static const QBrush b_continent3 = QBrush(QColor("peachpuff"));
        static const QBrush b_continent4 = QBrush(QColor("lavender"));
        static const QBrush b_continent5 = QBrush(QColor("aliceblue"));
        static const QBrush b_continent6 = QBrush(QColor("seashell"));
        static const QBrush b_continent7 = QBrush(QColor("tan"));

        for(int i = 0;i<ppHillList.size();++i){
            painter_png.drawPath(ppHillList[i]);
            painter_svg.drawPath(ppHillList[i]);
            if(i == 0){
                painter_png.fillPath(ppHillList[i],b_continent1);
                painter_svg.fillPath(ppHillList[i],b_continent1);
            }
            else if(i == 1){
                painter_png.fillPath(ppHillList[i],b_continent2);
                painter_svg.fillPath(ppHillList[i],b_continent2);
            }
            else if(i == 2){
                painter_png.fillPath(ppHillList[i],b_continent3);
                painter_svg.fillPath(ppHillList[i],b_continent3);
            }
            else if(3==i){
                painter_png.fillPath(ppHillList[i],b_continent4);
                painter_svg.fillPath(ppHillList[i],b_continent4);
            }
            else if(4 == i){
                painter_png.fillPath(ppHillList[i],b_continent5);
                painter_svg.fillPath(ppHillList[i],b_continent5);
            }
            else if(5 == i){
                painter_png.fillPath(ppHillList[i],b_continent6);
                painter_svg.fillPath(ppHillList[i],b_continent6);
            }
            else{
                painter_png.fillPath(ppHillList[i],b_continent7);
                painter_svg.fillPath(ppHillList[i],b_continent7);
            }
        }


    }
    else{
        for(int i = 0;i<pathList.size();++i){
            QList<Point*> & path = pathList[i];
            if(path.size() < 5){ //this branch should never run
                continue;
            }
            QPainterPath pp;
            getFilletPath(path,pp);
            if(isHill(path,pp,m_unit * 1.0 / shrinkScale)){
                if(ppHillList.size() >= 7) continue; //只保留七个大陆
                if(path.length() <= 5) continue; //太小的岛屿不要了,但是湖泊是需要的
                //hillList.push_back(path);
                ppHillList.push_back(pp);
            }
            else{ //所有的湖泊都是需要的
                //lakeList.push_back(path);
                ppLakeList.push_back(pp);
            }
        }
        for(int i = 0;i<ppHillList.size();++i){
            painter_png.drawPath(ppHillList[i]);
            painter_svg.drawPath(ppHillList[i]);
        }


    }
    /* 两种模式(线条,着色)下,湖泊都不需要填充颜色 */
    for(int i = 0;i<ppLakeList.size();++i){
        QPainterPath & pp = ppLakeList[i];
        bool insideLand = false;
        for(int j = 0;j<ppHillList.size();++j){
            if(ppHillList[j].contains(pp.pointAtPercent(0.1))){
                insideLand = true;
                break;
            }
        }
        if(!insideLand) continue; //不在七大陆中的湖泊就不要了

        painter_png.drawPath(ppLakeList[i]);
        painter_svg.drawPath(ppLakeList[i]);
    }
    painter_svg.end();
    image.save(g_path_mapResult + "map_" + QString::number(m_idxInFileName) + ".png");
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
QString Gen3DMapPoints::generateAMap(long long idx , bool fillColor,bool singleLand){
    m_fillColor = fillColor;
    m_singleLand = singleLand;
    m_idxInFileName = idx;
    onDrumClicked();
    QString ret = g_path_mapResult + "map_" + QString::number(idx) + ".svg";
    return ret;
}
Gen3DMapPoints::Gen3DMapPoints(bool noInit){
    m_fillColor = true;
    m_unit = 10000;
}
Gen3DMapPoints::Gen3DMapPoints()
{
    m_fillColor = true;
    m_unit = 10000;
    m_xPoint = 100 + 1;//81 points,80 edges
    m_yPoint = 50 + 1;//41个点，40条边
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
bool Gen3DMapPoints::hasScenario6(){
    QList<Point*> nextList;
    for(int i = 0;i<m_contourPointList.size();++i){
        Point * p = m_contourPointList[i];
        getPointsInSunShape(nextList,p,m_contourPointList);
        if(nextList.size() >= 6) return true;
    }
    return false;
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
                if(visit.count(path[i]) > 0) alreadySaved = true;
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
void Gen3DMapPoints::getContourPaths(QList< QList<Point*> > & pathList){
    set<Point*> visit;
    //第一步:先把所有钻石形的小洞找出来,每个小洞都是一个path
    findAllDiamonds(pathList,visit);
    printf("after finding all diamonds\n");
    QList<Point*> leftContourPointList = m_contourPointList;
    for(set<Point*>::iterator it = visit.begin();it!= visit.end();++it){
        leftContourPointList.removeOne(*it);
    }
    //printf("visit.size = %d,pathList.size = %d\n",visit.size(),pathList.size());
    //for(int i = 0;i<pathList.size();++i){
    //    printf("pathList[i].size = %d\n",pathList[i].size());
    //}
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
                if(0)
                {
                    QString data = "nextList.size = " + QString::number(nextList.size()) + "\npath:";
                    for(int i = 0;i< path.size();++i){
                        data += path[i]->str() + ";";
                    }
                    data += "\n";
                    QFile f("D:/QT/qtCreator/Test4AppMap/errPath.txt");
                    bool ok = f.open(QIODevice::Append);
                    if(ok){
                        f.write(data.toLocal8Bit());
                        f.close();
                    }
                }

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
        const set<Point*> & visit, const Point * center,const Point* prv){
    if(!center) return NULL;
    if(!prv) return NULL;
    bool onV = onVerticalBeam(center,m_unit);
    QList<Point*> resList;
    qDebug() << "center:" << center->str();
    qDebug() << "prv:" << prv->str();
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
                if((p->m_x - center->m_x) * (prv->m_x - center->m_x) < 0) resList.push_back(p);
            }
        }
        else{ //horizontal beam
            if(fabs(fabs(center->m_y - p->m_y) - m_unit) < 0.01){
                continue;
            }
            else {
                if((p->m_y - center->m_y) * (prv->m_y - center->m_y) < 0) resList.push_back(p);
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
    int maxz = 0;
    int minz = 0xfffffff;
    for(int i = 0;i<m_xPoint;++i){
        for(int j = 0;j<m_yPoint;++j){
            if(m_pArr[i][j]->m_z > maxz) maxz = m_pArr[i][j]->m_z;
            if(m_pArr[i][j]->m_z < minz) minz = m_pArr[i][j]->m_z;
        }
    }
    m_z = minz + (maxz - minz) / 4;
    //when m_xPoint*m_yPoint = about 5000 ,the amount of contour points this z cut out is about 28% of m_xPoint*m_yPoint
}


void Gen3DMapPoints::createArr(){
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
