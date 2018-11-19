#include "realisticrendering.h"
#include <QFileDialog>

RealisticRendering::RealisticRendering(QWidget *parent)
    : QMainWindow(parent), render(this)
{
    ui.setupUi(this);
    //this->setFixedSize(960, 640);
    createRenderArea();
    initOptions();
    initActions();
}

void RealisticRendering::createRenderArea() {
    ui.mainLayout->addWidget(&render);
    ui.mainLayout->setStretch(0, 1);
    ui.mainLayout->setStretch(1, 4);
}

void RealisticRendering::initOptions() {
    ui.phong->setChecked(true);
    ui.orthographic->setChecked(true);

    // connect signals
}

void RealisticRendering::initActions() {
    connect(ui.openScene, &QAction::triggered, this, [&]() {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open scene file"),
            QDir::homePath(), 
            tr("Scene File (*.scene)"));
        render.read_scene_file(fileName);
    });
}
