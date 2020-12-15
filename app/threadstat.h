#ifndef THREADSTAT_H
#define THREADSTAT_H
#include <QThread>
#include <QFile>
#include "public.h"
#include <QDateTime>
class ThreadStat:public QThread{
public:
    ThreadStat();
    bool m_todayChecked;
    long long m_yesterdayTotal;
    long long m_yesterdayGenMap;
    void stat();
    void run();
};

#endif // THREADSTAT_H
