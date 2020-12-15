/**
  @file
  @author Stefan Frings
*/
#include "startup.h"
using namespace stefanfrings;
#include <QTextCodec>
/**
  Entry point of the program.
*/
int main(int argc, char *argv[])
{
    // Use qtservice to start the application as a Windows service
    //…Ë÷√÷–Œƒ±‡¬Î
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));

    Startup startup(argc, argv);
    return startup.exec();
}
