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
    int m_xPoint;    //����������������
    int m_yPoint;    //����������������
    int m_unit; //gap unit
    int m_z; //suggest height

    enum CoastDirection{CoastNone = 0,CoastNorth = 1,CoastSouth = 2,CoastEast = 3,CoastWest = 4,CoastNorthEast = 5,
                       CoastNorthWest = 6,CoastSouthEast = 7,CoastSouthWest = 8,CoastFrame = 9 /* world map */};
    /* �����ߵ�λ�� */
    CoastDirection m_coastDir;

    /* �Ƿ���ʾ���� */
    bool m_showKingdom;

    /* �Ƿ������ɫ */
    bool m_fillColor;

    /* �����½���Ƕ���½ */
    bool m_singleLand;

    /* �Ƿ���ʾ���� */
    bool m_showRiver;

    /* �Ƿ���ʾ�¶ȴ� */
    bool m_showTemperature;

    /* �Ƿ���ʾɽ�� */
    bool m_showMountain;

    /* �����µ������ͼ , ����svg�ļ�·�� */
    QString generateNewWorld(long long idxInFileName);

    /* �����µľֲ���ͼ , ����svg�ļ�·�� */
    QString generateNewRegion(long long idxInFileName);

private:
    /* �����ж�ɽ����ʾ��qualified height */
    int m_mountainHeight;

    /* ���ɵ�����½ʱ��Ҫ�ѵ����½������ʾ,��ʱ��Ҫ��������ƽ�� */
    int m_translatex;
    int m_translatey;

    /* �����ͼƬ�ļ��е����к� */
    long long m_idxInFileName;

    /* �������꼯 */
    QList<QList<Point*> > m_pArr;//һ��  һ��  һ�� ...

    /* �Ĵ��ķ���,1:�����ó�¡��,-1:�����ó����� */
    int m_un;

    /* Ӱ�첨ϵ������ */
    QList<float> m_affectList;//1  0.5  0.25  0.13 Ӱ�첨��ϵ������

    /* ����ȸߵ㼯�� */
    QList<Point*> m_contourPointList;

    /* ���������ͼ */
    void createWorld();

    /* ���������ͼ */
    void createRegion();

    /* ��ȡһ����½�ϵ�����ݵ������,������kingdomList�� */
    void getKingdomRectList(QList<QRectF> & kingdomList,const QList<Point*> & path,const QPainterPath & pp,
                            int num = 5,int iconWidth = 60,int iconH = 60);
    /* �������ͼ�� */
    void drawKingdom(const QList<PathPainterPath> & pppHillList,QImage & png,QPainter & svgPainter);

    /* �滭�����½ */
    void drawLand(QList<QList<Point*> > & pathList,int shrinkScale,QImage & png,QPainter & svgPainter,
                  QSvgGenerator & svgGenerator,QList<PathPainterPath> & pppList,QList<PathPainterPath> & pppLakeList,
                  int landAmount);

    /* ��ȡ�ѵ����½�������������ϵ��ƫ��ֵ */
    void getShearOffset(QList<Point*> & path,int & x,int & y,int scale);

    /* �滭����½ */
    void drawLands(QList<QList<Point*> > & pathList,int shrinkScale,QImage & png,QPainter & svgPainter,
                QSvgGenerator & svgGenerator,QList<PathPainterPath> & pppHillList,QList<PathPainterPath> & pppLakeList,
                   int landAmount);

    /* ����png��svg */
    void output(QImage & png,QPainter & svgPainter,QString testPrefix = "");

    /* ��½��ɫ */
    void fillColorToLand(QImage & png,QPainter & svgPainter,QList<PathPainterPath> & pppHillList);

    /* ·���м��Ǻ��� */
    bool isLake(const QList<Point*> & path,const QPainterPath & pp,float gapUnit);

    /* ·���м���ɽ�� */
    bool isHill(const QList<Point*> & path,const QPainterPath & pp,float gapUnit);

    /* �������ĵ�Ͳ��ҵ�����Դ���ϻ�ȡ�����ĵ���������Ľ����ڽص����꼯�� */
    void getPointsInSunShape(QList<Point*> & pList,const Point* center,const QList<Point*> & srcList);

    /* get all the points that consist of a bunch of closed contours base on the height */
    void getContourPoints(QList<Point*> & pList,int height);

    /* ��m_contourPointList���ҵ����еıպ�·�� */
    void getContourPaths(QList< QList<Point*> > & pathList);

    void getContourPathsForRegion(QList< QList<Point*> > & pathList);

    Point* getContourPoint(const Point* node1,const Point* node2);

    Point* nextFrameNode(Point* cut);

    Point* nextFrameNode(Point* cur,QList<Point*> & pointList,std::set<Point*> visit);

    QList<Point*> nextFramePath(Point* cur,const QList<Point*>& srcList);

    void print(const QList<Point*> & path);

    /* ��m_contourPointList���ҵ����е�ֻ�����ĸ��ص��С�պ�����(��ʯ����),������pathList�� */
    void findAllDiamonds(QList< QList<Point*> > & pathList,set<Point*> & visit);

    /* judge if the point is part of some diamond,if so ,the four points of the diamond will be saved in path */
    bool consistDiamond(Point* p,QList<Point*> & path);

    /* ��������λ����,�ж�һ�����ǲ���������ı߿��� */
    bool onVerticalBeam(const Point* center,float unitGap);

    /* ��·����Сscale�� */
    void shrink(QList<QList<Point*> > & pathList,int scale);

    /* ������վ���ĳ�����,�������ĵ�,��ȡ������һ��ĳ����(������ĵ��������һ�����ĸ������) */
    Point* getIsolatePoint_sunStanding(const QList<Point*> & list,const Point* center);

    /* �����ֺ��ɵĳ�����,�������ĵ�,��ȡ������һ��ĳ����(������ĵ��������һ�����ĸ������) */
    Point* getIsolatePoint_sunLaying(const QList<Point*> & list,const Point* center);

    /* Ϊ���������ĸ����������ĵ�Ѱ����һ���ص� */
    Point* findNextPointForCenterWith4Points(const QList<Point*> & gateList,const set<Point*> & visit, const Point * center);

    /* Ϊ��������6�����������ĵ�Ѱ����һ���ص� */
    Point* findNextPointForCenterWith6Points(const QList<Point*> & gateList,const set<Point*> & visit, const Point * center,
                                             const Point* entrance);

    /* �������� */
    void createArr();

    /* ������б������ */
    void createTiltArr();

    /* �ͷ����������ڴ� */
    void clearArr();

    /*���������������õ�����Ĵ�С*/
    void setForce(int indexX,int indexY,int centerForce);

    /*ͬ��*/
    void setForce(Point* center,int centerForce);

    /*������������Ӱ�첨��ϵ������*/
    void setAffectPolicy(QList<float> & );

    /*ͬ��*/
    void setAffectPolicy(int levelAmount = 4);

    /*�Ĵ��㷨�Ľӿ�*/
    void drum(int times,float minForceFactor = 0.5,float maxForceFactor=2.0);

    /* ����ɳ��(�����������꼯�ϵı��) */
    void resetSandBox();

    /*����Ƽ��ĵȸ��߱��*/
    void getz();

    /* ��ȡ�����ǵ�QPainterPath */
    void getFilletPath(const QList<Point*> & path,QPainterPath & pp);

    /* ���ݵ�ǰ�ĺ����ڵ��ҵ���һ�������Ľڵ� */
    Point* getNextRiverNode(Point*  cur,int cut);

    /* ��ȡ�������� */
    void getRiverList(QList<QList<Point*> > & riverList);

    /* ���ƺ��� */
    void drawRiver(QList<QList<Point*> > & pathList,QImage & png,QPainter & svgPainter,QList<PathPainterPath> & landList);

    /* �����¶ȴ� */
    void drawTemperature(QImage & png,QPainter & svgPainter,int shrinkScale);

    /* ����ɽ�� */
    void drawMountain(const QList<PathPainterPath> & pppHillList,QImage & png,QPainter & svgPainter,int shrinkScale);

public:
    /******************************** TEST FUNCTION ********************************/
    /* ����ԭʼ�������� */
    void saveMeshData();

    /* ��ȡ�������� */
    void readMeshData();

    /* ������������ */
    void analyse();

    /* ��ʱ���Ժ��� */
    void test();

    /* ����·�� */
    void drawPath(QList<QList<Point*> > & pathList,QImage & png,QPainter & svgPainter);

    void drawSimpleLands(QList<QList<Point*> > & pathList,int shrinkScale,QImage & png,QPainter & svgPainter,
                QSvgGenerator & svgGenerator,QList<QPainterPath> & ppList);
private:
    /******************************** Math Function ********************************/
    /* �������������������Ҷ���,��ȡmid���cos()ֵ */
    double getCos(const Point* mid,const Point* prv,const Point* nxt);

    /* ���ݵ�ǰ��cur,��һ����nxt,���¸���far,��QPainterPath����һ��,��һ�ΰ���ֱ�߶κͻ��߶� */
    void extend(Point* cur,Point* nxt,Point* far,QPainterPath & pp);

    /* ���������������:(x1,y1),nxt,(x2,y2),����nxt���м�,����������������Բ��Բ������,������center�� */
    void getCenter(Point & center,float x1,float y1,float x2,float y2,const Point * nxt);

    /* ���������������:center��(arcX,arcY) ��ȡ����߶εĽǶ�,��centerΪ����ԭ�� */
    float getAngle(Point & center,float arcX,float arcY);

    /* ����·������,���ɱ���������˫���Ƶ㼯�� */
    void getDoubleBazierControls(QList<Point*>  & path,QList<Controls >  & control,float factor = 0.2);
    void getDoubleBazierControls(QList<QList<Point*> > & pathList,QList<QList<Controls > > & controlList,float factor = 0.2);

};

#endif // GEN3DMAPPOINTS_H
