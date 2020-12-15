#ifndef GEN3DMAPPOINTS_H
#define GEN3DMAPPOINTS_H
#include<iostream>
#include<sstream>
#include<set>
#include <QMainWindow>
#include <QPixmap>
#include <QGraphicsScene>
#include <QPainterPath>
using namespace std;
class Point{
public:
    Point();
    Point(float x,float y);
    QString toStr() const;
    QString str() const;
    float m_x;
    float m_y;
    float m_z;
    bool operator==(const Point& that);
    Point& operator=(const Point& that);
};
struct Controls{
    Point * m_c1;
    Point * m_c2;
};

class Gen3DMapPoints
{

public:
    explicit Gen3DMapPoints();
    explicit Gen3DMapPoints(bool noInit);
    ~Gen3DMapPoints();
    int m_xPoint;    //横坐标索引号
    int m_yPoint;    //纵坐标索引号
    int m_unit; //gap unit
    int m_z; //suggest height

    /* 生成一张地图 */
    QString generateAMap(long long idxInFileName,bool fillColor = true,bool singleLand = true);
    /**/
    void onDrumClicked();
private:
    bool m_fillColor;
    bool m_singleLand;
    long long m_idxInFileName;
    /* 网格坐标集 */
    QList<QList<Point*> > m_pArr;//一列  一列  一列 ...

    /* 影响波系数链表 */
    QList<float> m_affectList;//1  0.5  0.25  0.13 影响波的系数链表

    /* 单层等高点集合 */
    QList<Point*> m_contourPointList;

    void outputSingleLand(QList<QList<Point*> > & pathList,int shrinkScale);
    void getShearOffset(QList<Point*> & path,int & x,int & y,int scale);
    void output(QList<QList<Point*> > & pathList,int shrinkScale);

    /* 获取图例定位点 */
    QPointF getLegendAnchor(QList<Point*> & path,QPainterPath & pp,int txtLen);

    /* 给路径中的锐角倒角 */
    void fillet(QList<Point*> & path);

    /* 获取单个贝塞尔控制点 */
    void getSingleBazierControls(QList<QList<Point*> > & pathList,QList<QList<Point*> > & controlList);

    /* 获取两个贝塞尔控制点 */
    void getDoubleBazierControls(QList<QList<Point*> > & pathList,QList<QList<Controls > > & controlList);

    /* 根据一个截点和它下一个截点,获取这两个点中垂线上距离两个点中点一定距离的两个点,然后取这两个点中距离far远一些的那个店 */
    Point * getPerpendicularOffsetPoint(Point * cur,Point* nxt, Point* far);

    /* 路径中间是湖泊 */
    bool isLake(const QList<Point*> & path,const QPainterPath & pp,float gapUnit);

    /* 路径中间是山坡 */
    bool isHill(const QList<Point*> & path,const QPainterPath & pp,float gapUnit);

    /* 根据中心点和查找的坐标源集合获取该中心点的日字区的进出口截点坐标集合 */
    void getPointsInSunShape(QList<Point*> & pList,const Point* center,const QList<Point*> & srcList);

    /* get all the points that consist of a bunch of closed contours base on the height */
    void getContourPoints(QList<Point*> & pList,int height);

    /* 从m_contourPointList中找到所有的闭合路径 */
    void getContourPaths(QList< QList<Point*> > & pathList);

    /* 从m_contourPointList中找到所有的只含有四个截点的小闭合区域(钻石区域),保存在pathList中 */
    void findAllDiamonds(QList< QList<Point*> > & pathList,set<Point*> & visit);

    /* judge if the point is part of some diamond,if so ,the four points of the diamond will be saved in path */
    bool consistDiamond(Point* p,QList<Point*> & path);

    /* 判断m_contourPointList中是否有六个截点在同一个日字区域的场景 */
    bool hasScenario6();

    /* 根据网格单位长度,判断一个点是不是在竖向的边框上 */
    bool onVerticalBeam(const Point* center,float unitGap);

    /* 把路径缩小scale倍 */
    void shrink(QList<QList<Point*> > & pathList,int scale);

    /* 在日字站立的场景下,根据中心点,获取孤立在一侧的出入点(这个中心点的日字区一定有四个出入点) */
    Point* getIsolatePoint_sunStanding(const QList<Point*> & list,const Point* center);

    /* 在日字横躺的场景下,根据中心点,获取孤立在一侧的出入点(这个中心点的日字区一定有四个出入点) */
    Point* getIsolatePoint_sunLaying(const QList<Point*> & list,const Point* center);

    /* 为日字区有四个出入点的中心点寻找下一个截点 */
    Point* findNextPointForCenterWith4Points(const QList<Point*> & gateList,const set<Point*> & visit, const Point * center);

    /* 为日字区有6个出入点的中心点寻找下一个截点,目前没有用 */
    Point* findNextPointForCenterWith6Points(const QList<Point*> & gateList,const set<Point*> & visit, const Point * center,
                                             const Point* prv);

    /*初始化*/
    void createArr();

    /*设置作用力的作用点和力的大小*/
    void setForce(int indexX,int indexY,int centerForce);

    /*同上*/
    void setForce(Point* center,int centerForce);

    /*设置作用力的影响波的系数分配*/
    void setAffectPolicy(QList<float> & );

    /*同上*/
    void setAffectPolicy(int levelAmount = 4);

    /*鼓锤算法的接口*/
    void drum(int times,float force=2.0);

    /* 重置沙盘(重置网格坐标集合的标高) */
    void resetSandBox();

    /*获得推荐的等高线标高*/
    void getz();
public:
    /******************************** TEST FUNCTION ********************************/
    void saveContourPath(QList<QList<Point*> > & pathList);
    void saveMeshData();
    void readMeshData();
    void analyse();
    void getContourPathList(QList<QList<Point*> > & pathList);
};

#endif // GEN3DMAPPOINTS_H
