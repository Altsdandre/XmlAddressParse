#include "XmlAddressParse.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtWebEngine::initialize();
    XmlAddressParse w;
    w.show();
    return a.exec();
}
