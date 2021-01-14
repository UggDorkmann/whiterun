#ifndef _POINT_H_
#define _POINT_H_
#include <QDebug>
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
#endif