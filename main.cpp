#include "XmlAddressParse.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    XmlAddressParse w;
    w.show();
    return a.exec();
}
