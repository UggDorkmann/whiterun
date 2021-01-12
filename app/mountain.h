#ifndef MOUNTAIN_H
#define MOUNTAIN_H
#include <QImage>
#include <QPainter>

class Mountain : public QImage
{
public:
    Mountain(int w,int h,QImage::Format fmt = QImage::Format_ARGB32);
};

#endif // MOUNTAIN_H
