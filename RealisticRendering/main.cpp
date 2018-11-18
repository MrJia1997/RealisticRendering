#include "realisticrendering.h"
#include <QtWidgets/QApplication>
#include "scene.h"

int main(int argc, char *argv[])
{
    scene s;
    s.read_scene_file("scene/Scene_1.scene");
    QApplication a(argc, argv);
    RealisticRendering w;
    w.show();
    return a.exec();
}
