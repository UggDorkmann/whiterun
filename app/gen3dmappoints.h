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

    /* ��ȡһ����½�ϵ�����ݵ������,������kingdomList�� */
    void getKingdomRectList(QList<QRectF> & kingdomList,QList<Point*> & path,QPainterPath & pp,
                            int num = 5,int iconWidth = 60);

    /* ��������½ͼ */
    void outputSingleLand(QList<QList<Point*> > & pathList,int shrinkScale);

    /* ��ȡ�ѵ����½�������������ϵ��ƫ��ֵ */
    void getShearOffset(QList<Point*> & path,int & x,int & y,int scale);

    /* �����½ͼ */
    void output(QList<QList<Point*> > & pathList,int shrinkScale);

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

    void getFilletPath(const QList<Point*> & path,QPainterPath & pp);
public:
    /******************************** TEST FUNCTION ********************************/
    void saveContourPath(QList<QList<Point*> > & pathList);
    void saveMeshData();
    void readMeshData();
    void analyse();
    void getContourPathList(QList<QList<Point*> > & pathList);

private:
    /******************************** Math Function ********************************/
    double getCos(Point* mid,Point* prv,Point* nxt);
    void cutObtuse(Point* cur,Point* prv,Point* nxt);
    void extend(Point* cur,Point* nxt,Point* far,QPainterPath & pp);
    void getCenter(Point & center,float x1,float y1,float x2,float y2,const Point * nxt);
    float getAngle(Point & center,float arcX,float arcY);
};

#endif // GEN3DMAPPOINTS_H
