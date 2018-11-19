#include "renderingwidget.h"
#include "input.h"

// Front Verticies
#define VERTEX_FTR vertex( vec3f( 0.5f,  0.5f,  0.5f), vec4f( 1.0f, 0.0f, 0.0f, 1.0f ) )
#define VERTEX_FTL vertex( vec3f(-0.5f,  0.5f,  0.5f), vec4f( 0.0f, 1.0f, 0.0f, 1.0f ) )
#define VERTEX_FBL vertex( vec3f(-0.5f, -0.5f,  0.5f), vec4f( 0.0f, 0.0f, 1.0f, 1.0f ) )
#define VERTEX_FBR vertex( vec3f( 0.5f, -0.5f,  0.5f), vec4f( 0.0f, 0.0f, 0.0f, 1.0f ) )

// Back Verticies
#define VERTEX_BTR vertex( vec3f( 0.5f,  0.5f, -0.5f), vec4f( 1.0f, 1.0f, 0.0f, 1.0f ) )
#define VERTEX_BTL vertex( vec3f(-0.5f,  0.5f, -0.5f), vec4f( 0.0f, 1.0f, 1.0f, 1.0f ) )
#define VERTEX_BBL vertex( vec3f(-0.5f, -0.5f, -0.5f), vec4f( 1.0f, 0.0f, 1.0f, 1.0f ) )
#define VERTEX_BBR vertex( vec3f( 0.5f, -0.5f, -0.5f), vec4f( 1.0f, 1.0f, 1.0f, 1.0f ) )

// Create a colored cube
static const std::vector<vertex> vs({
    // Face 1 (Front)
    VERTEX_FTR, VERTEX_FTL, VERTEX_FBL,
    VERTEX_FBL, VERTEX_FBR, VERTEX_FTR,
    // Face 2 (Back)
    VERTEX_BBR, VERTEX_BTL, VERTEX_BTR,
    VERTEX_BTL, VERTEX_BBR, VERTEX_BBL,
    // Face 3 (Top)
    VERTEX_FTR, VERTEX_BTR, VERTEX_BTL,
    VERTEX_BTL, VERTEX_FTL, VERTEX_FTR,
    // Face 4 (Bottom)
    VERTEX_FBR, VERTEX_FBL, VERTEX_BBL,
    VERTEX_BBL, VERTEX_BBR, VERTEX_FBR,
    // Face 5 (Left)
    VERTEX_FBL, VERTEX_FTL, VERTEX_BTL,
    VERTEX_FBL, VERTEX_BTL, VERTEX_BBL,
    // Face 6 (Right)
    VERTEX_FTR, VERTEX_FBR, VERTEX_BBR,
    VERTEX_BBR, VERTEX_BTR, VERTEX_FTR
});

#undef VERTEX_BBR
#undef VERTEX_BBL
#undef VERTEX_BTL
#undef VERTEX_BTR

#undef VERTEX_FBR
#undef VERTEX_FBL
#undef VERTEX_FTL
#undef VERTEX_FTR

RenderingWidget::RenderingWidget(QWidget *parent) 
    : QOpenGLWidget(parent), pScene(nullptr) {
    this->grabKeyboard();

    mTransform.translate(0.0f, 0.0f, -5.0f);
}

RenderingWidget::~RenderingWidget() {
    if (pScene != nullptr)
        delete pScene;
}

void RenderingWidget::read_scene_file(QString fileName) {
    if (pScene != nullptr)
        delete pScene;

    pScene = new scene;
    pScene->read_scene_file(fileName.toStdString());
}

void RenderingWidget::initializeGL() {
    initializeOpenGLFunctions();
    connect(this, &QOpenGLWidget::frameSwapped, this, &RenderingWidget::update);
    
    glClearColor(0.5, 0.5, 0.5, 0.0);
    glEnable(GL_CULL_FACE);
    /*glShadeModel(GL_SMOOTH);
    glEnable(GL_DOUBLEBUFFER);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1);*/


    {
        // Create Shader (Do not release until VAO is created)
        mProgram = new QOpenGLShaderProgram();
        mProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/simple.vert");
        mProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/simple.frag");
        mProgram->link();
        mProgram->bind();

        // Cache Uniform Locations
        uModelToWorld = mProgram->uniformLocation("modelToWorld");
        uWorldToCamera = mProgram->uniformLocation("worldToCamera");
        uCameraToView = mProgram->uniformLocation("cameraToView");

        // Create Buffer (Do not release until VAO is created)
        mVertex.create();
        mVertex.bind();
        mVertex.setUsagePattern(QOpenGLBuffer::StaticDraw);
        mVertex.allocate(&vs[0], vs.size() * sizeof(vertex));

        // Create Vertex Array Object
        mObject.create();
        mObject.bind();
        mProgram->setAttributeBuffer(0, GL_FLOAT, offsetof(vertex, position), 3, sizeof(vertex));
        mProgram->setAttributeBuffer(1, GL_FLOAT, offsetof(vertex, color),  4, sizeof(vertex));
        mProgram->enableAttributeArray(0);
        mProgram->enableAttributeArray(1);

        // Release (unbind) all
        mObject.release();
        mVertex.release();
        mProgram->release();
    }
}

void RenderingWidget::resizeGL(int w, int h) {
    mProjection.setToIdentity();
    mProjection.perspective(45.0f, w / static_cast<float>(h), 0.0f, 1000.0f);
}

void RenderingWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    mProgram->bind();
    mProgram->setUniformValue(uWorldToCamera, mCamera.toMatrix());
    mProgram->setUniformValue(uCameraToView, mProjection);
    {
        mObject.bind();
        mProgram->setUniformValue(uModelToWorld, mTransform.toMatrix());
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vs.size()));
        mObject.release();
    }
    mProgram->release();
}

void RenderingWidget::teardownGL() {
    mObject.destroy();
    mVertex.destroy();
    delete mProgram;
}

void RenderingWidget::update() {
    // Update input
    Input::update();

    // Camera Transformation
    static const float transSpeed = 0.2f;
    static const float rotSpeed = 0.2f;
    
    if (Input::buttonPressed(Qt::RightButton))
    {
        // Handle rotations
        mCamera.rotate(-rotSpeed * Input::mouseDelta().x(), Camera3D::LocalUp);
        mCamera.rotate(-rotSpeed * Input::mouseDelta().y(), mCamera.right());
    }
    
    // Handle translations
    QVector3D translation;
    if (Input::keyPressed(Qt::Key_W))
    {
        translation += mCamera.up();
    }
    if (Input::keyPressed(Qt::Key_S))
    {
        translation -= mCamera.up();
    }
    if (Input::keyPressed(Qt::Key_A))
    {
        translation -= mCamera.right();
    }
    if (Input::keyPressed(Qt::Key_D))
    {
        translation += mCamera.right();
    }
    if (Input::keyPressed(Qt::Key_Q))
    {
        translation -= mCamera.forward();
    }
    if (Input::keyPressed(Qt::Key_E))
    {
        translation += mCamera.forward();
    }
    mCamera.translate(transSpeed * translation);

    mTransform.rotate(1.0f, QVector3D(0.4f, 0.3f, 0.3f));

    QOpenGLWidget::update();
}

void RenderingWidget::keyPressEvent(QKeyEvent *event) {
    
    int a = 0;
    if (event->isAutoRepeat()) {
        event->ignore();
    }
    else {
        Input::registerKeyPress(event->key());
    }
}

void RenderingWidget::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) {
        event->ignore();
    }
    else {
        Input::registerKeyRelease(event->key());
    }
}

void RenderingWidget::mousePressEvent(QMouseEvent *event) {
    Input::registerMousePress(event->button());
}

void RenderingWidget::mouseReleaseEvent(QMouseEvent *event) {
    Input::registerMouseRelease(event->button());
}
