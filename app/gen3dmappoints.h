#ifndef GEN3DMAPPOINTS_H
#define GEN3DMAPPOINTS_H
#include<iostream>
#include<sstream>
#include<set>
#include <QMainWindow>
#include <QPixmap>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QSvgGenerator>
using namespace std;
class Point{
public:
    explicit Point();
    explicit Point(float x,float y);
    explicit Point(float x,float y,float z);
    QString toStr() const;
    QString str() const;
    float m_x;
    float m_y;
    float m_z;
    bool operator==(const Point& that)const;
    Point& operator=(const Point& that);
};
class PathPainterPath{
public:
    QList<Point*> m_path;
    QPainterPath m_pp;
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
    explicit Gen3DMapPoints(int xPoints,int yPoints);
    ~Gen3DMapPoints();
    int m_xPoint;    //横坐标网格点的数量
    int m_yPoint;    //纵坐标网格点的数量
    int m_unit; //gap unit
    int m_z; //suggest height

    enum CoastDirection{CoastNone = 0,CoastNorth = 1,CoastSouth = 2,CoastEast = 3,CoastWest = 4,CoastNorthEast = 5,
                       CoastNorthWest = 6,CoastSouthEast = 7,CoastSouthWest = 8,CoastFrame = 9 /* world map */};
    /* 海岸线的位置 */
    CoastDirection m_coastDir;

    /* 是否显示诸侯国 */
    bool m_showKingdom;

    /* 是否填充颜色 */
    bool m_fillColor;

    /* 单块大陆还是多块大陆 */
    bool m_singleLand;

    /* 是否显示河流 */
    bool m_showRiver;

    /* 是否显示温度带 */
    bool m_showTemperature;

    /* 是否显示山脉 */
    bool m_showMountain;

    /* 生成新的世界地图 , 返回svg文件路径 */
    QString generateNewWorld(long long idxInFileName);

    /* 生成新的局部地图 , 返回svg文件路径 */
    QString generateNewRegion(long long idxInFileName);

private:
    /* 用于判断山脉显示的qualified height */
    int m_mountainHeight;

    /* 生成单个大陆时需要把单块大陆居中显示,这时需要把坐标轴平移 */
    int m_translatex;
    int m_translatey;

    /* 保存的图片文件中的序列号 */
    long long m_idxInFileName;

    /* 网格坐标集 */
    QList<QList<Point*> > m_pArr;//一列  一列  一列 ...

    /* 鼓锤的方向,1:向上敲出隆块,-1:向下敲出凹坑 */
    int m_un;

    /* 影响波系数链表 */
    QList<float> m_affectList;//1  0.5  0.25  0.13 影响波的系数链表

    /* 单层等高点集合 */
    QList<Point*> m_contourPointList;

    /* 生成世界地图 */
    void createWorld();

    /* 生成区域地图 */
    void createRegion();

    /* 获取一个大陆上的诸侯割据点的链表,保存在kingdomList中 */
    void getKingdomRectList(QList<QRectF> & kingdomList,const QList<Point*> & path,const QPainterPath & pp,
                            int num = 5,int iconWidth = 60,int iconH = 60);
    /* 添加诸侯国图标 */
    void drawKingdom(const QList<PathPainterPath> & pppHillList,QImage & png,QPainter & svgPainter);

    /* 绘画单块大陆 */
    void drawLand(QList<QList<Point*> > & pathList,int shrinkScale,QImage & png,QPainter & svgPainter,
                  QSvgGenerator & svgGenerator,QList<PathPainterPath> & pppList,QList<PathPainterPath> & pppLakeList,
                  int landAmount);

    /* 获取把单块大陆居中所需的坐标系的偏移值 */
    void getShearOffset(QList<Point*> & path,int & x,int & y,int scale);

    /* 绘画多块大陆 */
    void drawLands(QList<QList<Point*> > & pathList,int shrinkScale,QImage & png,QPainter & svgPainter,
                QSvgGenerator & svgGenerator,QList<PathPainterPath> & pppHillList,QList<PathPainterPath> & pppLakeList,
                   int landAmount);

    /* 保存png和svg */
    void output(QImage & png,QPainter & svgPainter,QString testPrefix = "");

    /* 大陆着色 */
    void fillColorToLand(QImage & png,QPainter & svgPainter,QList<PathPainterPath> & pppHillList);

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

    void getContourPathsForRegion(QList< QList<Point*> > & pathList);

    Point* getContourPoint(const Point* node1,const Point* node2);

    Point* nextFrameNode(Point* cut);

    Point* nextFrameNode(Point* cur,QList<Point*> & pointList,std::set<Point*> visit);

    QList<Point*> nextFramePath(Point* cur,const QList<Point*>& srcList);

    void print(const QList<Point*> & path);

    /* 从m_contourPointList中找到所有的只含有四个截点的小闭合区域(钻石区域),保存在pathList中 */
    void findAllDiamonds(QList< QList<Point*> > & pathList,set<Point*> & visit);

    /* judge if the point is part of some diamond,if so ,the four points of the diamond will be saved in path */
    bool consistDiamond(Point* p,QList<Point*> & path);

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

    /* 为日字区有6个出入点的中心点寻找下一个截点 */
    Point* findNextPointForCenterWith6Points(const QList<Point*> & gateList,const set<Point*> & visit, const Point * center,
                                             const Point* entrance);

    /* 创建网格 */
    void createArr();

    /* 创建倾斜的网格 */
    void createTiltArr();

    /* 释放网格坐标内存 */
    void clearArr();

    /*设置作用力的作用点和力的大小*/
    void setForce(int indexX,int indexY,int centerForce);

    /*同上*/
    void setForce(Point* center,int centerForce);

    /*设置作用力的影响波的系数分配*/
    void setAffectPolicy(QList<float> & );

    /*同上*/
    void setAffectPolicy(int levelAmount = 4);

    /*鼓锤算法的接口*/
    void drum(int times,float minForceFactor = 0.5,float maxForceFactor=2.0);

    /* 重置沙盘(重置网格坐标集合的标高) */
    void resetSandBox();

    /*获得推荐的等高线标高*/
    void getz();

    /* 获取倒过角的QPainterPath */
    void getFilletPath(const QList<Point*> & path,QPainterPath & pp);

    /* 根据当前的河流节点找到下一个流经的节点 */
    Point* getNextRiverNode(Point*  cur,int cut);

    /* 获取河流集合 */
    void getRiverList(QList<QList<Point*> > & riverList);

    /* 绘制河流 */
    void drawRiver(QList<QList<Point*> > & pathList,QImage & png,QPainter & svgPainter,QList<PathPainterPath> & landList);

    /* 绘制温度带 */
    void drawTemperature(QImage & png,QPainter & svgPainter,int shrinkScale);

    /* 绘制山脉 */
    void drawMountain(const QList<PathPainterPath> & pppHillList,QImage & png,QPainter & svgPainter,int shrinkScale);

public:
    /******************************** TEST FUNCTION ********************************/
    /* 保存原始网格数据 */
    void saveMeshData();

    /* 读取网格数据 */
    void readMeshData();

    /* 分析网格数据 */
    void analyse();

    /* 临时测试函数 */
    void test();

    /* 绘制路径 */
    void drawPath(QList<QList<Point*> > & pathList,QImage & png,QPainter & svgPainter);

    void drawSimpleLands(QList<QList<Point*> > & pathList,int shrinkScale,QImage & png,QPainter & svgPainter,
                QSvgGenerator & svgGenerator,QList<QPainterPath> & ppList);
private:
    /******************************** Math Function ********************************/
    /* 根据三个点的坐标和余弦定理,获取mid点的cos()值 */
    double getCos(const Point* mid,const Point* prv,const Point* nxt);

    /* 根据当前点cur,下一个点nxt,下下个点far,把QPainterPath延伸一段,这一段包含直线段和弧线段 */
    void extend(Point* cur,Point* nxt,Point* far,QPainterPath & pp);

    /* 根据三个点的坐标:(x1,y1),nxt,(x2,y2),其中nxt在中间,获得这三个点的外切圆的圆心坐标,保存在center中 */
    void getCenter(Point & center,float x1,float y1,float x2,float y2,const Point * nxt);

    /* 根据两个点的坐标:center和(arcX,arcY) 获取这个线段的角度,以center为坐标原点 */
    float getAngle(Point & center,float arcX,float arcY);

    /* 根据路径集合,生成贝塞尔曲线双控制点集合 */
    void getDoubleBazierControls(QList<Point*>  & path,QList<Controls >  & control,float factor = 0.2);
    void getDoubleBazierControls(QList<QList<Point*> > & pathList,QList<QList<Controls > > & controlList,float factor = 0.2);

};

#endif // GEN3DMAPPOINTS_H
