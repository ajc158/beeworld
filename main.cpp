#include <QApplication>
#include "beeworldwindow.h"

int main(int argc, char *argv[])
{
#ifndef IS_COMMANDLINE
    QApplication a(argc, argv);
#else
    QCoreApplication a(argc, argv);
#endif
    QString file = "";
    uint port = 50091;
    if (a.arguments().size() == 3) {
        file = a.arguments().at(1);
        QString portT = a.arguments().at(2);
        port = portT.toInt();
    }
    BeeWorldWindow w(file, port);
#ifndef IS_COMMANDLINE
    w.show();
#endif
    
    return a.exec();
}
