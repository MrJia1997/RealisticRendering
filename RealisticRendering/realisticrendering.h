#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_realisticrendering.h"
#include "renderingwidget.h"

class RealisticRendering : public QMainWindow
{
    Q_OBJECT

public:
    RealisticRendering(QWidget *parent = Q_NULLPTR);

private:
    void createRenderArea();
    void initOptions();
    void initActions();

private:
    Ui::RealisticRenderingClass ui;
    RenderingWidget render;


};
