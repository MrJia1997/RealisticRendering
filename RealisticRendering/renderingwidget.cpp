#include "renderingwidget.h"
#include "input.h"

RenderingWidget::RenderingWidget(QWidget *parent) 
    : QOpenGLWidget(parent), 
    pScene(nullptr),
    mProgram(nullptr),
    mTexture(nullptr),
    mDisplacement(nullptr),
    //mFBO(nullptr),
    projType(PERSPECTIVE),
    orthoRange(1.5f),
    drawArraySize(0) {
    
    this->grabKeyboard();

    lightPosition = QVector4D(0.0f, 3.0f, 0.0f, 1.0f);
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
    mVertex.allocate(&rawData[0], rawData.size() * sizeof(vertex));
    drawArraySize = rawData.size();
    mObject.bind();

    // Release all
    mObject.release();
    mVertex.release();

    mVertexShadow.bind();
    mVertexShadow.allocate(&rawData[0], rawData.size() * sizeof(vertex));
    mObjectShadow.bind();

    mObjectShadow.release();
    mVertexShadow.release();

}

void RenderingWidget::initializeGL() {
    initializeOpenGLFunctions();
    connect(this, &QOpenGLWidget::frameSwapped, this, &RenderingWidget::update);

    glClearColor(0.5, 0.5, 0.5, 0.0);
    glClearDepth(1);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);

    
    glEnable(GL_DOUBLEBUFFER);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Create common shader
    setupShaderProgram("shaders/phong.vert", "shaders/phong.frag");
    mVertex.create();
    mVertex.bind();
    mVertex.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    mObject.create();
    mObject.bind();
    
    mProgram->setAttributeBuffer(0, GL_FLOAT, offsetof(vertex, position), 3, sizeof(vertex));
    mProgram->setAttributeBuffer(1, GL_FLOAT, offsetof(vertex, normal), 3, sizeof(vertex));
    mProgram->setAttributeBuffer(2, GL_FLOAT, offsetof(vertex, texCoord), 3, sizeof(vertex));
    mProgram->enableAttributeArray(0);
    mProgram->enableAttributeArray(1);
    mProgram->enableAttributeArray(2);

    // Load Texture
    load_texture("texture/marble.jpg");
    load_displacement("texture/rock/Rock_DISPLACEMENT.png");

    // Release (unbind) all
    mObject.release();
    mVertex.release();
    mProgram->release();

    // Create shadow
    setupShadowProgram("shaders/depth.vert", "shaders/depth.frag");
    mVertexShadow.create();
    mVertexShadow.bind();
    mVertexShadow.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    mObjectShadow.create();
    mObjectShadow.bind();

    mShadow->setAttributeBuffer(0, GL_FLOAT, offsetof(vertex, position), 3, sizeof(vertex));
    mShadow->enableAttributeArray(0);

    load_FBO();

    mObjectShadow.release();
    mVertexShadow.release();
    mShadow->release();
}

void RenderingWidget::resizeGL(int w, int h) {
    mProjection.setToIdentity();
    float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    if (projType == PERSPECTIVE)
        mProjection.perspective(45.0f, aspectRatio, 0.1f, 1000.0f);
    else if (projType == ORTHOGRAPHIC)
        mProjection.ortho(
            -orthoRange * aspectRatio, 
            orthoRange * aspectRatio, 
            -orthoRange, 
            orthoRange, 
            0.0f, 1000.0f);
}

void RenderingWidget::paintGL() {
    renderShadow();
    renderObject();
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

void RenderingWidget::setupShadowProgram(const char * vertFile, const char * fragFile) {
    // Create Shader (Do not release until VAO is created)
    mShadow = new QOpenGLShaderProgram();
    mShadow->addShaderFromSourceFile(QOpenGLShader::Vertex, vertFile);
    mShadow->addShaderFromSourceFile(QOpenGLShader::Fragment, fragFile);
    mShadow->link();
}

void RenderingWidget::renderShadow() {
    QMatrix4x4 lightViewMat;
    lightViewMat.setToIdentity();
    lightViewMat.lookAt(lightPosition.toVector3D(),
        QVector3D(0, 0, 0),
        QVector3D(0, 1, 0));
    
    QMatrix4x4 projMat;
    int w = this->width(), h = this->height();
    projMat.setToIdentity();
    float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    projMat.perspective(90.0f, aspectRatio, 0.1f, 1000.0f);
    
    QMatrix4x4 lightViewProjMat = projMat * lightViewMat;

    //mFBO->bind();
    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadowFramebuffer);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glShadeModel(GL_SMOOTH);
    //glCullFace(GL_FRONT);
    mShadow->bind();
    mVertexShadow.bind();

    mShadow->setUniformValue("lightViewProjMat", lightViewProjMat);
    mShadow->setUniformValue("modelMat", mTransform.toMatrix());

    mObjectShadow.bind();
    glDrawArrays(GL_TRIANGLES, 0, drawArraySize);
    mObjectShadow.release();

    mVertexShadow.release();
    mShadow->release();
    //glCullFace(GL_BACK);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, w, h);
}

void RenderingWidget::renderObject() {
    QMatrix4x4 lightViewMat;
    lightViewMat.setToIdentity();
    lightViewMat.lookAt(lightPosition.toVector3D(),
        QVector3D(0, 0, 0),
        QVector3D(0, 1, 0));

    QMatrix4x4 projMat;
    int w = this->width(), h = this->height();
    projMat.setToIdentity();
    float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    projMat.perspective(90.0f, aspectRatio, 0.1f, 1000.0f);

    QMatrix4x4 lightViewProjMat = projMat * lightViewMat;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glShadeModel(GL_SMOOTH);

    mProgram->bind();
    mVertex.bind();
    mTexture->bind(0);
    mDisplacement->bind(1);
    
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glActiveTexture(GL_TEXTURE2);
    mProgram->setUniformValue("shadowUnit", 2);

    mProgram->setUniformValue("viewMat", mCamera.toMatrix());
    mProgram->setUniformValue("projection", mProjection);
    mProgram->setUniformValue("normalMat", mTransform.toMatrix().normalMatrix());
    mProgram->setUniformValue("lightViewProjMat", lightViewProjMat);

    mProgram->setUniformValue("material.Kd", 0.5f, 0.5f, 0.5f);
    mProgram->setUniformValue("light.Ld", 1.0f, 1.0f, 1.0f);
    mProgram->setUniformValue("light.Position", mCamera.toMatrix() * lightPosition);
    mProgram->setUniformValue("material.Ka", 0.5f, 0.5f, 0.5f);
    mProgram->setUniformValue("light.La", 0.7f, 0.7f, 0.7f);
    mProgram->setUniformValue("material.Ks", 0.8f, 0.8f, 0.8f);
    mProgram->setUniformValue("light.Ls", 1.0f, 1.0f, 1.0f);
    mProgram->setUniformValue("material.Shininess", 60.0f);

    mObject.bind();
    mProgram->setUniformValue("modelMat", mTransform.toMatrix());
    glDrawArrays(GL_TRIANGLES, 0, drawArraySize);
    mObject.release();

    mTexture->release();
    mDisplacement->release();
    mVertex.release();
    mProgram->release();
}

void RenderingWidget::load_texture(QString fileName) {
    if (mTexture != nullptr)
        delete mTexture;

    mTexture = new QOpenGLTexture(QImage(fileName).mirrored());
    mTexture->setMinificationFilter(QOpenGLTexture::Linear);
    mTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    mTexture->setWrapMode(QOpenGLTexture::Repeat);
    glActiveTexture(GL_TEXTURE0);
    mProgram->setUniformValue("texUnit", 0);    
}

void RenderingWidget::load_displacement(QString fileName) {
    if (mDisplacement != nullptr)
        delete mDisplacement;

    mDisplacement = new QOpenGLTexture(QImage(fileName).mirrored());
    mDisplacement->setMinificationFilter(QOpenGLTexture::Linear);
    mDisplacement->setMagnificationFilter(QOpenGLTexture::Linear);
    mDisplacement->setWrapMode(QOpenGLTexture::Repeat);
    glActiveTexture(GL_TEXTURE1);
    mProgram->setUniformValue("dispUnit", 1);
}

void RenderingWidget::load_FBO() {
    /*if (mFBO != nullptr)
        delete mFBO;

    mFBO = new QOpenGLFramebufferObject(1024, 1024, 
        QOpenGLFramebufferObject::Depth);
    */

    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glActiveTexture(GL_TEXTURE2);
    mProgram->setUniformValue("shadowUnit", 2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 1024, 1024,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // GL_CLAMP_TO_EDGE setups the shadow map in such a way that
    // fragments for which the shadow map is undefined
    // will get values from closest edges of the shadow map
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // comparison mode of the shadow map
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glGenFramebuffers(1, &shadowFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadowFramebuffer);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, shadowMap, 0);

    glDrawBuffer(GL_NONE);

}

void RenderingWidget::update() {
    // Update input
    Input::update();

    // Camera Transformation
    static const float transSpeed = 0.2f;
    static const float rotSpeed = 0.2f;
    
    if (Input::buttonPressed(Qt::LeftButton))
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

    if (translation != QVector3D(0.0, 0.0, 0.0)) {
        Camera3D tempCam = mCamera;
        tempCam.translate(transSpeed * translation);
        
        Point outside(0.0, 0.0, 1000.0);
        QMatrix4x4 camCoord = tempCam.toMatrix().inverted();
        Point cam(camCoord(0, 3), camCoord(1, 3), camCoord(2, 3));
        
        bool isColliding = false;

        for (auto t : pScene->aabbTrees) {
            double distance = t->tree.squared_distance(cam);
            if (distance <= 0.3)
                isColliding = true;
        }
        
        
        if (!isColliding) {
            mCamera = tempCam;
        }
    }
        
    //mCamera.translate(transSpeed * translation);
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