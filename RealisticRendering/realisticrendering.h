#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_realisticrendering.h"

class RealisticRendering : public QMainWindow
{
    Q_OBJECT

public:
    RealisticRendering(QWidget *parent = Q_NULLPTR);

private:
    Ui::RealisticRenderingClass ui;
};
