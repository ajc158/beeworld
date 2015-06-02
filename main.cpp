#include <QApplication>
#include "beeworldwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BeeWorldWindow w;
    w.show();
    
    return a.exec();
}
