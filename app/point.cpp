#include "point.h"
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