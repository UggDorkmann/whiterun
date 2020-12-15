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
    int m_xPoint;    //������������
    int m_yPoint;    //������������
    int m_unit; //gap unit
    int m_z; //suggest height

    /* ����һ�ŵ�ͼ */
    QString generateAMap(long long idxInFileName,bool fillColor = true,bool singleLand = true);
    /**/
    void onDrumClicked();
private:
    bool m_fillColor;
    bool m_singleLand;
    long long m_idxInFileName;
    /* �������꼯 */
    QList<QList<Point*> > m_pArr;//һ��  һ��  һ�� ...

    /* Ӱ�첨ϵ������ */
    QList<float> m_affectList;//1  0.5  0.25  0.13 Ӱ�첨��ϵ������

    /* ����ȸߵ㼯�� */
    QList<Point*> m_contourPointList;

    void outputSingleLand(QList<QList<Point*> > & pathList,int shrinkScale);
    void getShearOffset(QList<Point*> & path,int & x,int & y,int scale);
    void output(QList<QList<Point*> > & pathList,int shrinkScale);

    /* ��ȡͼ����λ�� */
    QPointF getLegendAnchor(QList<Point*> & path,QPainterPath & pp,int txtLen);

    /* ��·���е���ǵ��� */
    void fillet(QList<Point*> & path);

    /* ��ȡ�������������Ƶ� */
    void getSingleBazierControls(QList<QList<Point*> > & pathList,QList<QList<Point*> > & controlList);

    /* ��ȡ�������������Ƶ� */
    void getDoubleBazierControls(QList<QList<Point*> > & pathList,QList<QList<Controls > > & controlList);

    /* ����һ���ص������һ���ص�,��ȡ���������д����Ͼ����������е�һ�������������,Ȼ��ȡ���������о���farԶһЩ���Ǹ��� */
    Point * getPerpendicularOffsetPoint(Point * cur,Point* nxt, Point* far);

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

    /* ��m_contourPointList���ҵ����е�ֻ�����ĸ��ص��С�պ�����(��ʯ����),������pathList�� */
    void findAllDiamonds(QList< QList<Point*> > & pathList,set<Point*> & visit);

    /* judge if the point is part of some diamond,if so ,the four points of the diamond will be saved in path */
    bool consistDiamond(Point* p,QList<Point*> & path);

    /* �ж�m_contourPointList���Ƿ��������ص���ͬһ����������ĳ��� */
    bool hasScenario6();

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

    /* Ϊ��������6�����������ĵ�Ѱ����һ���ص�,Ŀǰû���� */
    Point* findNextPointForCenterWith6Points(const QList<Point*> & gateList,const set<Point*> & visit, const Point * center,
                                             const Point* prv);

    /*��ʼ��*/
    void createArr();

    /*���������������õ�����Ĵ�С*/
    void setForce(int indexX,int indexY,int centerForce);

    /*ͬ��*/
    void setForce(Point* center,int centerForce);

    /*������������Ӱ�첨��ϵ������*/
    void setAffectPolicy(QList<float> & );

    /*ͬ��*/
    void setAffectPolicy(int levelAmount = 4);

    /*�Ĵ��㷨�Ľӿ�*/
    void drum(int times,float force=2.0);

    /* ����ɳ��(�����������꼯�ϵı��) */
    void resetSandBox();

    /*����Ƽ��ĵȸ��߱��*/
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
