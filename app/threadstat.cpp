#include "threadstat.h"
extern long long g_visitCnt;
extern long long g_mapGenTimes;
ThreadStat::ThreadStat(){
    m_todayChecked = false;
    m_yesterdayTotal = 0;
    m_yesterdayGenMap = 0;
}
void ThreadStat::stat(){
    QFile f(g_path_stat);
    bool ok = f.open(QIODevice::Append);
    if(ok){
        QString data = QDateTime::currentDateTime().toString() + "\n";
        data += "Total: " + QString::number(g_visitCnt) + ",Yesterday: " +
                QString::number(g_visitCnt - m_yesterdayTotal) + ",GenMapTime: " +
                QString::number(g_mapGenTimes) + ",Yesterday GenMapTime: " +
                QString::number(g_mapGenTimes - m_yesterdayGenMap) + "\n\n";
        f.write(data.toLocal8Bit());
        f.close();
        m_yesterdayTotal = g_visitCnt;
        m_yesterdayGenMap = g_mapGenTimes;
    }
}

void ThreadStat::run(){
    printf("stat thread is running\n");
    while(1){
        QDateTime dt = QDateTime::currentDateTime();
        long long secs = dt.toSecsSinceEpoch();
        int leftsecs = secs % 86400;
        if(leftsecs < 3600){//新的一天的前一个小时
            if(!m_todayChecked){
                stat();
                m_todayChecked = true;
            }
            else{
                this->sleep(1);//sleep 1 hour
            }
        }
        else{
            if(leftsecs > 86400-7200){
                //还有不到2小时就到凌晨0点了
                m_todayChecked = false;
            }
            this->sleep(3000);//sleep 50 minutes
            printf("I am sleeping for 50 minutes\n");
        }
    }
}
