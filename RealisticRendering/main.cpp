#include "realisticrendering.h"
#include <QtWidgets/QApplication>
#include "scene.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RealisticRendering w;
    w.show();
    return a.exec();
}
