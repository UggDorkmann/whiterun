#ifndef PUBLIC_H
#define PUBLIC_H
#include <QDebug>


#ifdef WIN32
    #define M_DuringTest 1
    const QString g_projectAPath = "D:/shensheng/_ToRep/ProjectA";

#else
    #define M_DuringTest 0
    const QString g_projectAPath = "/root/whiterun/front/ProjectA";
#endif
    const QString g_path_mapResult = g_projectAPath + "/result_maps/";
    const QString g_path_test =      g_projectAPath + "/test/";
    const QString g_path_data = g_projectAPath + "/data/";
    const QString g_ath_eng_ch_dic = g_path_data + "_19k_eng_ch.txt";
    const QString g_wordListPath = g_path_data + "chineseWordsList.txt";
    const QString g_singleFirstChineseNameListPath = g_path_data + "singleFirstNameList.txt";
    const QString g_doubleFirstChineseNameListPath = g_path_data + "doubleFirstNameList.txt";
    const QString g_maleNamePath = g_path_data + "top_1000_maleNames_english.txt";
    const QString g_femaleNamePath = g_path_data + "top_1000_femaleNames_english.txt";
    const QString g_firstEnglishNamePath = g_path_data + "top_1000_firstNames_english.txt";
    const QString g_phraseNPath = g_path_data + "phraseList_n.txt";
    const QString g_phraseAdjPath = g_path_data + "phraseList_adj.txt";
    const QString g_path_stat = g_path_data + "myStat.txt";
#endif // PUBLIC_H
