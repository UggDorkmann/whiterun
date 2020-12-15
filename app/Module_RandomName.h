#ifndef MODULE_NICKNAME_H
#define MODULE_NICKNAME_H
#include <QDebug>
#include <set>
class Module_RandomName
{
public:
    static Module_RandomName & getInstance();
    static void clearInstance();

    enum NameElement{
        NS_All = 0 ,
        NS_N = 1,
        NS_V = 2,
        NS_Adj = 3,
        NS_O = 4,
        NS_NULL = 5
    };
    enum FirstNameType{
        FNT_Single = 1,
        FNT_Double = 2
    };
    //nickname
    void getNickName(QStringList & resList,int amount = 5);

    //Chinese names

    void getLongNames(QStringList & resList,NameElement ne1 = NS_V,NameElement ne2 = NS_N, int amount=5,
                      FirstNameType fnt = FNT_Single);

    //English names
    void getMaleNames(QStringList & resList,int amount=5);
    void getFemaleNames(QStringList & resList, int amount=5);
private:
    static Module_RandomName * sm_randName;
    Module_RandomName();
    //nickname
    QStringList m_phrase_n;
    QStringList m_phrase_adj;
    QStringList m_phrase_vt;  //useless right now
    QStringList m_phrase_vi;  //useless right now
    QStringList m_phrase_v;   //useless right now
    void loadPhrase();
    void readDic();
    void procData();

    //words
    QStringList m_words_n; //noun
    QStringList m_words_adj; //adjective
    QStringList m_words_v;//verb
    QStringList m_words_o;//other

    QStringList m_singleFirstNameList;
    QStringList m_doubleFirstNameList;
    void readChineseWordList();
    void readFirstNameList();

    QString getRandomWordFromAllList();
    QString getWordBy(const NameElement & ne);

    //English names
    QStringList m_maleNameList;
    QStringList m_femaleNameList;
    QStringList m_englishFirstNameList;

    void readEnglishNameData();
};

#endif // MODULE_NICKNAME_H
