#pragma once

#include "scene.h"
#include "transform3D.h"
#include "camera3D.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>

class QOpenGLShaderProgram;

enum {
    ORTHOGRAPHIC,
    PERSPECTIVE
};

class RenderingWidget : public QOpenGLWidget, 
    protected QOpenGLFunctions {

    Q_OBJECT

public:
    RenderingWidget(QWidget *parent);
    ~RenderingWidget();

    void read_scene_file(QString fileName);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void teardownGL();

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    
protected slots:
    void update();

public slots:
    void set_proj_type(int type);

private:
    scene *pScene;

    QOpenGLBuffer mVertex;
    QOpenGLVertexArrayObject mObject;
    QOpenGLShaderProgram *mProgram;

    int uModelToWorld;
    int uWorldToCamera;
    int uCameraToView;

    int projType;
    float orthoRange;

    QMatrix4x4 mProjection;
    Camera3D mCamera;
    Transform3D mTransform;
};