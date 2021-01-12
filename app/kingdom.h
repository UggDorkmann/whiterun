#ifndef KINGDOM_H
#define KINGDOM_H
#include <QImage>
#include <QPainter>
class Kingdom : public QImage
{
public:
    Kingdom(int w,int h,QImage::Format fmt = QImage::Format_ARGB32);

};

#endif // KINGDOM_H
