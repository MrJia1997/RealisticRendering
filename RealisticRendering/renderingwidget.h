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
#include <QOpenGLTexture>
#include <QOpenGLFrameBufferObject>

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>

class QOpenGLShaderProgram;

enum {
    ORTHOGRAPHIC,
    PERSPECTIVE
};

enum {
    POINT_LIGHT,
    DIRECTIONAL_LIGHT
};

struct Light {
    float La;       // Ambient light intensity
    float Ld;       // Diffuse light intensity
    float Ls;       // Specular light intensity
    QVector3D Position;
    QVector3D Direction;
    int Type;           // Point or directional light

    Light() : La(1.0), Ld(1.0), Ls(1.0), 
        Position(QVector3D(0.0, 3.0, 0.0)), Type(POINT_LIGHT)  {}
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
    
    void setupShaderProgram(const char *vertFile, const char *fragFile);
    void setupShadowProgram(const char *vertFile, const char *fragFile);

    void renderShadow();
    void renderObject();

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    
protected slots:
    void update();

public slots:
    void set_proj_type(int type);
    void load_texture(QString fileName);
    void load_displacement(QString fileName);
    void load_FBO();

    QColor trace(Ray ray, int depth, Light light);
    void renderObjectRayTracing(Light light);

private:
    scene *pScene;
    std::vector<vertex> rawData;
    int drawArraySize;

    QOpenGLBuffer mVertex;
    QOpenGLVertexArrayObject mObject;
    QOpenGLShaderProgram *mProgram;

    QOpenGLBuffer mVertexShadow;
    QOpenGLVertexArrayObject mObjectShadow;
    QOpenGLShaderProgram *mShadow;
    
    QOpenGLTexture *mTexture;
    QOpenGLTexture *mDisplacement;
    
    //QOpenGLFramebufferObject *mFBO;
    GLuint shadowMap;
    GLuint shadowFramebuffer;

    int projType;
    float orthoRange;

    QVector4D lightPosition;
    QMatrix4x4 mProjection;
    Camera3D mCamera;
    Transform3D mTransform;
};