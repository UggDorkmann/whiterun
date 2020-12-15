#include "Module_RandomName.h"
#include <QDebug>
#include <QFile>
#include <QRegExp>
#include <ctime>
#include <stdlib.h>
#include "public.h"

Module_RandomName * Module_RandomName::sm_randName = NULL;
Module_RandomName & Module_RandomName::getInstance(){
    if(sm_randName == NULL){
        sm_randName = new Module_RandomName;
    }
    return *sm_randName;
}
void Module_RandomName::clearInstance(){
    if(sm_randName){
        delete sm_randName;
        sm_randName = NULL;
    }
}
Module_RandomName::Module_RandomName()
{
    srand(time(0));

    //readDic();
    //procData();
    loadPhrase();

    readChineseWordList();//汉字3800个
    readFirstNameList();//百家姓

    readEnglishNameData();
}
void Module_RandomName::readEnglishNameData(){
    QFile f(g_firstEnglishNamePath);
    bool ok = f.open(QIODevice::ReadOnly);
    if(ok){
        QString data = QString::fromLocal8Bit(f.readAll());
        data.replace("\r\n","\n");
        m_englishFirstNameList = data.split("\n");
        while(m_englishFirstNameList.contains("")) m_englishFirstNameList.removeOne("");
        f.close();
    }
    QFile f2(g_maleNamePath);
    QFile f3(g_femaleNamePath);
    ok = f2.open(QIODevice::ReadOnly);
    if(ok){
        QString data = QString::fromLocal8Bit(f2.readAll());
        data.replace("\r\n","\n");
        m_maleNameList = data.split("\n");
        while(m_maleNameList.contains("")) m_maleNameList.removeOne("");
        f2.close();
    }
    ok = f3.open(QIODevice::ReadOnly);
    if(ok){
        QString data = QString::fromLocal8Bit(f3.readAll());
        data.replace("\r\n","\n");
        m_femaleNameList = data.split("\n");
        while(m_femaleNameList.contains("")) m_femaleNameList.removeOne("");
        f3.close();
    }
}
void Module_RandomName::loadPhrase(){
    QFile f_n(g_phraseNPath);
    bool ok = f_n.open(QIODevice::ReadOnly);
    if(ok){
        QString data = QString::fromLocal8Bit(f_n.readAll());
        data.replace("\r\n","\n");
        m_phrase_n = data.split("\n",QString::SkipEmptyParts);
        while(m_phrase_n.contains("")) m_phrase_n.removeOne("");
        f_n.close();
        for(int i = 0;i<m_phrase_n.size();++i){
            QString & str = m_phrase_n[i];
            if(str.endsWith(QString::fromLocal8Bit("的"))){
                qDebug() << str;
            }
        }
    }
    QFile f_adj(g_phraseAdjPath);
    ok = f_adj.open(QIODevice::ReadOnly);
    if(ok){
        QString data = QString::fromLocal8Bit(f_adj.readAll());
        data.replace("\r\n","\n");
        m_phrase_adj = data.split("\n",QString::SkipEmptyParts);
        while(m_phrase_adj.contains("")) m_phrase_adj.removeOne("");
        f_adj.close();
    }
}
void Module_RandomName::readDic(){
    QRegExp rx("(?=n\\.)|(?=adj\\.)|(?=vt\\.)|(?=vi\\.)|(?=v\\.)"); //保留分隔符
    QRegExp rx_bracket("\\(.*\\)");
    QRegExp rx_angledBracket("<.*>");
    QRegExp rx_squareBracket("\\[.*\\]");
    QRegExp rx_chineseBracket(QString::fromLocal8Bit("（.*）"));
    QFile f(g_ath_eng_ch_dic);
    bool ok = f.open(QIODevice::ReadOnly);
    if(ok){
        QString data = QString::fromUtf8(f.readAll());
        data.replace("\r\n","\n");
        QStringList dataList = data.split("\n");
        QString str;
        QStringList tmpList;
        for(int i = 0;i<dataList.size();++i){
            str = dataList[i];
            tmpList = str.split("----");
            if(tmpList.size() < 2) continue;
            str = tmpList[1];
            tmpList = str.split(rx,QString::SkipEmptyParts);
            for(int j = 0;j<tmpList.size();++j){

                QString & s = tmpList[j];
                if(s.contains(QString::fromLocal8Bit("的复数"))) continue;
                if(s.contains(QString::fromLocal8Bit("的过去式"))) continue;
                if(s.contains(QString::fromLocal8Bit("的过去分词"))) continue;
                if(s.contains("...")) continue;
                if(s.contains(QString::fromLocal8Bit("公司"))) continue;
                if(s.contains("art.")) s = s.split("art.").at(0);
                if(s.contains("num.")) s = s.split("num.").at(0);
                if(s.contains("int.")) s = s.split("int.").at(0);
                if(s.contains("abbr."))s = s.split("abbr.").at(0);
                if(s.contains("conj."))s = s.split("conj.").at(0);
                if(s.contains("prep."))s = s.split("prep.").at(0);

                s.replace(rx_bracket,"");
                s.replace(rx_angledBracket,"");
                s.replace(rx_squareBracket,"");
                s.replace(QRegExp("ad$"),"");
                s.replace(QRegExp("pro$"),"");
                s.replace(rx_chineseBracket,"");

                s.replace(QString::fromLocal8Bit("！"),"");
                s.replace("!","");

                if(s.startsWith("n.")){
                    s = s.split("n.").at(1);
                    m_phrase_n += s.split(",",QString::SkipEmptyParts);
                }
                else if(s.startsWith("adj.")){

                    s = s.split("adj.").at(1);
                    m_phrase_adj += s.split(",",QString::SkipEmptyParts);
                }
                //else if(s.startsWith("vt.")){
                //    s = s.split("vt.").at(1);
                //    m_list_vt += s.split(",",QString::SkipEmptyParts);
                //}
                //else if(s.startsWith("vi.")){
                //    s = s.split("vi.").at(1);
                //    m_list_vi += s.split(",",QString::SkipEmptyParts);
                //}
                //else if(s.startsWith("v.")){
                //    s = s.split("v.").at(1);
                //    m_list_v += s.split(",",QString::SkipEmptyParts);
                //}
            }

        }
        qDebug() << "n list size = " << m_phrase_n.size();
        qDebug() << "adj list size = " << m_phrase_adj.size();
        //qDebug() << "vt list size = " << m_phrase_vt.size();
        //qDebug() << "vi list size = " << m_phrase_vi.size();
        //qDebug() << "v list size = " << m_phrase_v.size();

        f.close();

    }
}
static void checkData(QStringList & strList ,QList<int> & badList ,bool debug = false){
    bool badWord;
    badList.clear();
    for(int i = 0;i<strList.size();++i){
        badWord = false;
        QString & str = strList[i];
        for(int j = 0;j< str.length();++j){
            if(str.at(j).unicode() < 256){
                badWord = true;
                break;
            }
        }
        if(badWord){
            badList.push_front(i);
            if(debug) qDebug() << str;
        }
    }
}
void Module_RandomName::readChineseWordList(){
    QFile f(g_wordListPath);
    bool ok = f.open(QIODevice::ReadOnly);
    if(ok){
        QString data = QString::fromLocal8Bit(f.readAll());
        data.replace("\r\n","\n");
        QStringList dataList = data.split("\n");
        QString word;
        QString rest;
        for(int i = 0;i<dataList.size();++i){
            QString & str = dataList[i];
            if(!str.contains(":")) continue;
            word = str.split(":").at(0);
            rest = str.split(":").at(1);
            if(rest.contains("adj")){
                m_words_adj.push_back(word);
            }
            if(rest.contains("n")){
                m_words_n.push_back(word);
            }
            if(rest.contains("v")){
                m_words_v.push_back(word);
            }
            if(rest.contains("o")){
                m_words_o.push_back(word);
            }
        }
        qDebug() << "m_words_adj.size = " << m_words_adj.size();
        qDebug() << "m_words_n.size = " << m_words_n.size();
        qDebug() << "m_words_v.size = " << m_words_v.size();
        qDebug() << "m_words_o.size = " << m_words_o.size();
        f.close();
    }
}
void Module_RandomName::readFirstNameList(){
    {
        QFile f(g_singleFirstChineseNameListPath);
        bool ok = f.open(QIODevice::ReadOnly);
        if(ok){
            QString data = QString::fromLocal8Bit(f.readAll());
            data.replace("\r\n","\n");
            m_singleFirstNameList = data.split("\n");
            while(m_singleFirstNameList.contains("")) m_singleFirstNameList.removeOne("");
            f.close();
        }
    }
    {
        QFile f2(g_doubleFirstChineseNameListPath);
        bool ok = f2.open(QIODevice::ReadOnly);
        if(ok){
            QString data = QString::fromUtf8(f2.readAll());
            data.replace("\r\n","\n");
            m_doubleFirstNameList = data.split("\n");
            while(m_doubleFirstNameList.contains("")) m_doubleFirstNameList.removeOne("");
            f2.close();
        }
    }

}
void Module_RandomName::procData(){
    QList<int> badList;
    checkData(m_phrase_n,badList);
    qDebug() << "badList.size = " << badList.size();
    for(int i = 0;i<badList.size();++i){
        m_phrase_n.removeAt(badList[i]);
    }
    checkData(m_phrase_adj,badList,true);
    qDebug() << "badList.size = " << badList.size();
    for(int i = 0;i<badList.size();++i){
        m_phrase_adj.removeAt(badList[i]);
    }
    QFile f_adj("./out_adj.txt");

    f_adj.open(QIODevice::WriteOnly);
    f_adj.write(m_phrase_adj.join("\n").toLocal8Bit());

    f_adj.close();
}

//nickname
void Module_RandomName::getNickName(QStringList & resList,int amount){
    srand(time(0));
    resList.clear();
    int size1 = m_phrase_adj.size();
    int size2 = m_phrase_n.size();
    QString str;
    for(int i = 0;i< amount ; ++i){
        str = m_phrase_adj[rand() % size1] + m_phrase_n[rand() % size2];
        resList << str;
    }
}

void Module_RandomName::getLongNames(QStringList & resList,NameElement ne1,NameElement ne2, int amount
                                     ,FirstNameType fnt){
    srand(time(0));
    resList.clear();
    static const int size_1first = m_singleFirstNameList.size();
    static const int size_2first = m_doubleFirstNameList.size();
    QString str1,str2;
    for(int i = 0;i<amount; ++i){
        str1 = getWordBy(ne1);
        str2 = getWordBy(ne2);
        if(fnt == FNT_Single){
            resList << m_singleFirstNameList[rand() % size_1first] + str1 + str2;
        }
        else{
            resList << m_doubleFirstNameList[rand() % size_2first] + str1 + str2;
        }

    }
}

QString Module_RandomName::getWordBy(const NameElement & ne){
    static const int size_n = m_words_n.size();
    static const int size_v = m_words_v.size();
    static const int size_o = m_words_o.size();
    static const int size_adj = m_words_adj.size();
    QString str;
    if(ne == NS_N) str = m_words_n[rand() % size_n];
    else if(ne == NS_O) str = m_words_o[rand() % size_o];
    else if(ne == NS_V) str = m_words_v[rand() % size_v];
    else if(ne == NS_Adj) str = m_words_adj[rand() % size_adj];
    else if(ne == NS_All) str = getRandomWordFromAllList();
    else if(ne == NS_NULL) str = "";
    return str;
}
QString Module_RandomName::getRandomWordFromAllList(){
    static const int size_n = m_words_n.size();
    static const int size_v = m_words_v.size();
    static const int size_o = m_words_o.size();
    static const int size_adj = m_words_adj.size();
    static const int size_all = size_n + size_o + size_v + size_adj;

    int idx = rand() % size_all;
    if(idx < size_n){
        return m_words_n[idx];
    }
    if(idx < size_n + size_v){
        return m_words_v[idx - size_n];
    }
    if(idx < size_n + size_v + size_o){
        return m_words_o[idx - size_n - size_v];
    }
    return m_words_adj[idx - size_n - size_v - size_o];
}
//English names
void Module_RandomName::getMaleNames(QStringList & resList,int amount){
    srand(time(0));
    resList.clear();
    static const int size_first = m_englishFirstNameList.size();
    static const int size_male = m_maleNameList.size();
    for(int i = 0;i<amount;++i){
        resList << m_maleNameList[rand() % size_male] + " " + m_englishFirstNameList[rand() % size_first];
    }
}
void Module_RandomName::getFemaleNames(QStringList & resList, int amount){
    srand(time(0));
    resList.clear();
    static const int size_first = m_englishFirstNameList.size();
    static const int size_fe = m_femaleNameList.size();
    for(int i = 0;i<amount;++i){
        resList << m_femaleNameList[rand() % size_fe] + " " + m_englishFirstNameList[rand() % size_first];
    }
}
