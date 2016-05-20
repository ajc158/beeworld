#include <QApplication>
#include "beeworldwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString file = "";
    uint port = 50091;
    if (a.arguments().size() == 3) {
        file = a.arguments().at(1);
        QString portT = a.arguments().at(2);
        port = portT.toInt();
    }
    BeeWorldWindow w(file, port);
    w.show();
    
    return a.exec();
}
