#include "Application.h"

int main(int argc, char *argv[])
{
#ifndef DISABLE_GUI
    // NOTE: must be set before QCoreApplication is created
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    Application a(argc, argv);
	
	//TODO: initialize configure or something

    return a.exec();
}
