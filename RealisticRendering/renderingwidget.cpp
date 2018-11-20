#include "renderingwidget.h"
#include "input.h"

// Front Verticies
#define VERTEX_FTR_F vertex( vec3f( 0.5f,  0.5f,  0.5f), vec3f( 0.0f, 0.0f, 1.0f) )
#define VERTEX_FTR_T vertex( vec3f( 0.5f,  0.5f,  0.5f), vec3f( 0.0f, 1.0f, 0.0f) )
#define VERTEX_FTR_R vertex( vec3f( 0.5f,  0.5f,  0.5f), vec3f( 1.0f, 0.0f, 0.0f) )
#define VERTEX_FTL_F vertex( vec3f(-0.5f,  0.5f,  0.5f), vec3f( 0.0f, 0.0f, 1.0f) )
#define VERTEX_FTL_T vertex( vec3f(-0.5f,  0.5f,  0.5f), vec3f( 0.0f, 1.0f, 0.0f) )
#define VERTEX_FTL_L vertex( vec3f(-0.5f,  0.5f,  0.5f), vec3f(-1.0f, 0.0f, 0.0f) )
#define VERTEX_FBL_F vertex( vec3f(-0.5f, -0.5f,  0.5f), vec3f( 0.0f, 0.0f, 1.0f) )
#define VERTEX_FBL_B vertex( vec3f(-0.5f, -0.5f,  0.5f), vec3f( 0.0f,-1.0f, 0.0f) )
#define VERTEX_FBL_L vertex( vec3f(-0.5f, -0.5f,  0.5f), vec3f(-1.0f, 0.0f, 0.0f) )
#define VERTEX_FBR_F vertex( vec3f( 0.5f, -0.5f,  0.5f), vec3f( 0.0f, 0.0f, 1.0f) )
#define VERTEX_FBR_B vertex( vec3f( 0.5f, -0.5f,  0.5f), vec3f( 0.0f,-1.0f, 0.0f) )
#define VERTEX_FBR_R vertex( vec3f( 0.5f, -0.5f,  0.5f), vec3f( 1.0f, 0.0f, 0.0f) )

// Back Verticies
#define VERTEX_BTR_B vertex( vec3f( 0.5f,  0.5f, -0.5f), vec3f( 0.0f, 0.0f,-1.0f) )
#define VERTEX_BTR_T vertex( vec3f( 0.5f,  0.5f, -0.5f), vec3f( 0.0f, 1.0f, 0.0f) )
#define VERTEX_BTR_R vertex( vec3f( 0.5f,  0.5f, -0.5f), vec3f( 1.0f, 0.0f, 0.0f) )
#define VERTEX_BTL_B vertex( vec3f(-0.5f,  0.5f, -0.5f), vec3f( 0.0f, 0.0f,-1.0f) )
#define VERTEX_BTL_T vertex( vec3f(-0.5f,  0.5f, -0.5f), vec3f( 0.0f, 1.0f, 0.0f) )
#define VERTEX_BTL_L vertex( vec3f(-0.5f,  0.5f, -0.5f), vec3f(-1.0f, 0.0f, 0.0f) )
#define VERTEX_BBL_BA vertex( vec3f(-0.5f, -0.5f, -0.5f), vec3f( 0.0f, 0.0f,-1.0f) )
#define VERTEX_BBL_BO vertex( vec3f(-0.5f, -0.5f, -0.5f), vec3f( 0.0f,-1.0f, 0.0f) )
#define VERTEX_BBL_L vertex( vec3f(-0.5f, -0.5f, -0.5f), vec3f(-1.0f, 0.0f, 0.0f) )
#define VERTEX_BBR_BA vertex( vec3f( 0.5f, -0.5f, -0.5f), vec3f( 0.0f, 0.0f,-1.0f) )
#define VERTEX_BBR_BO vertex( vec3f( 0.5f, -0.5f, -0.5f), vec3f( 0.0f,-1.0f, 0.0f) )
#define VERTEX_BBR_R vertex( vec3f( 0.5f, -0.5f, -0.5f), vec3f( 1.0f, 0.0f, 0.0f) )


// Create a colored cube
static const std::vector<vertex> vs({
    // Face 1 (Front)
    VERTEX_FTR_F, VERTEX_FTL_F, VERTEX_FBL_F,
    VERTEX_FBL_F, VERTEX_FBR_F, VERTEX_FTR_F,
    // Face 2 (Back)
    VERTEX_BBR_BA, VERTEX_BTL_B, VERTEX_BTR_B,
    VERTEX_BTL_B, VERTEX_BBR_BA, VERTEX_BBL_BA,
    // Face 3 (Top)
    VERTEX_FTR_T, VERTEX_BTR_T, VERTEX_BTL_T,
    VERTEX_BTL_T, VERTEX_FTL_T, VERTEX_FTR_T,
    // Face 4 (Bottom)
    VERTEX_FBR_B, VERTEX_FBL_B, VERTEX_BBL_BO,
    VERTEX_BBL_BO, VERTEX_BBR_BO, VERTEX_FBR_B,
    // Face 5 (Left)
    VERTEX_FBL_L, VERTEX_FTL_L, VERTEX_BTL_L,
    VERTEX_FBL_L, VERTEX_BTL_L, VERTEX_BBL_L,
    // Face 6 (Right)
    VERTEX_FTR_R, VERTEX_FBR_R, VERTEX_BBR_R,
    VERTEX_BBR_R, VERTEX_BTR_R, VERTEX_FTR_R
});

static const std::vector<vertex> vs2({
    // Face 1 (Front)
    VERTEX_FTR_F, VERTEX_FTL_F, VERTEX_FBL_F,
    VERTEX_FBL_F, VERTEX_FBR_F, VERTEX_FTR_F,
});

RenderingWidget::RenderingWidget(QWidget *parent) 
    : QOpenGLWidget(parent), 
    pScene(nullptr),
    mProgram(nullptr),
    projType(PERSPECTIVE),
    orthoRange(1.5f),
    drawArraySize(0) {
    
    this->grabKeyboard();

    mCamera.translate(0.0f, 0.0f, 5.0f);
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

    rawData.clear();
    for (auto o : pScene->objects) {
        std::vector<vertex> data = o->raw_data();
        rawData.insert(rawData.end(), data.begin(), data.end());
    }

    if (rawData.empty())
        return;

    // Update Buffer
    mVertex.bind();
    mVertex.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVertex.allocate(&rawData[0], rawData.size() * sizeof(vertex));
    drawArraySize = rawData.size();
    mObject.bind();

    // Release all
    mObject.release();
    mVertex.release();
}

void RenderingWidget::initializeGL() {
    initializeOpenGLFunctions();
    connect(this, &QOpenGLWidget::frameSwapped, this, &RenderingWidget::update);
    
    glClearColor(0.5, 0.5, 0.5, 0.0);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DOUBLEBUFFER);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    //glEnable(GL_DEPTH_TEST);
    //glClearDepth(1);

    setupShaderProgram("shaders/phong.vert", "shaders/phong.frag");
    setupBuffer();
    setupVAO();
    
    mProgram->setAttributeBuffer(0, GL_FLOAT, offsetof(vertex, position), 3, sizeof(vertex));
    mProgram->setAttributeBuffer(1, GL_FLOAT, offsetof(vertex, normal), 3, sizeof(vertex));
    mProgram->setAttributeBuffer(2, GL_FLOAT, offsetof(vertex, color), 4, sizeof(vertex));
    mProgram->enableAttributeArray(0);
    mProgram->enableAttributeArray(1);
    mProgram->enableAttributeArray(2);

    // Release (unbind) all
    mObject.release();
    mVertex.release();
    mProgram->release();
}

void RenderingWidget::resizeGL(int w, int h) {
    mProjection.setToIdentity();
    float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    if (projType == PERSPECTIVE)
        mProjection.perspective(45.0f, aspectRatio, 0.0f, 1000.0f);
    else if (projType == ORTHOGRAPHIC)
        mProjection.ortho(
            -orthoRange * aspectRatio, 
            orthoRange * aspectRatio, 
            -orthoRange, 
            orthoRange, 
            0.0f, 1000.0f);
}

void RenderingWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    mProgram->bind();

    mProgram->setUniformValue("viewMat", mCamera.toMatrix());
    mProgram->setUniformValue("projection", mProjection);
    mProgram->setUniformValue("normalMat", mTransform.toMatrix().normalMatrix());
     
    QVector4D worldLight(5.0f, 5.0f, 2.0f, 1.0f);
    mProgram->setUniformValue("material.Kd", 0.9f, 0.5f, 0.3f);
    mProgram->setUniformValue("light.Ld", 1.0f, 1.0f, 1.0f);
    mProgram->setUniformValue("light.Position", mCamera.toMatrix() * worldLight);
    mProgram->setUniformValue("material.Ka", 0.9f, 0.5f, 0.3f);
    mProgram->setUniformValue("light.La", 0.4f, 0.4f, 0.4f);
    mProgram->setUniformValue("material.Ks", 0.8f, 0.8f, 0.8f);
    mProgram->setUniformValue("light.Ls", 1.0f, 1.0f, 1.0f);
    mProgram->setUniformValue("material.Shininess", 100.0f);

    {
        mObject.bind();
        mProgram->setUniformValue("modelMat", mTransform.toMatrix());
        glDrawArrays(GL_TRIANGLES, 0, drawArraySize);
        mObject.release();
    }
    mProgram->release();
}

void RenderingWidget::teardownGL() {
    mObject.destroy();
    mVertex.destroy();
    if (mProgram != nullptr)
        delete mProgram;
}

void RenderingWidget::setupShaderProgram(const char *vertFile, const char *fragFile) {
    // Create Shader (Do not release until VAO is created)
    mProgram = new QOpenGLShaderProgram();
    mProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, vertFile);
    mProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, fragFile);
    mProgram->link();
    
}

void RenderingWidget::setupBuffer() {
    // Create Buffer (Do not release until VAO is created)
    mVertex.create();
    mVertex.bind();
    mVertex.setUsagePattern(QOpenGLBuffer::StaticDraw);
}

void RenderingWidget::setupVAO() {
    // Create Vertex Array Object
    mObject.create();
    mObject.bind();
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
        translation += mCamera.forward();
    }
    if (Input::keyPressed(Qt::Key_E))
    {
        translation -= mCamera.forward();
    }
    mCamera.translate(transSpeed * translation);

    //mTransform.rotate(1.0f, QVector3D(0.4f, 0.3f, 0.3f));

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

void RenderingWidget::set_proj_type(int type) {
    projType = type;
    int w = this->width(), h = this->height();
    resizeGL(w, h);
}