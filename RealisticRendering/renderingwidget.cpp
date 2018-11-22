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

    for (int i = 0; i < pScene->objects.size(); i++) {
        if (i < pScene->objects.size() - 1)
            pScene->objects[i]->material.Type = REFLECTION_AND_REFRACTION;
        else
            pScene->objects[i]->material.Type = DIFFUSE_AND_GLOSSY;
        
    }
}

void RenderingWidget::initializeGL() {
    initializeOpenGLFunctions();
    connect(this, &QOpenGLWidget::frameSwapped, this, &RenderingWidget::update);

    glClearColor(0, 0, 0, 0.0);
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

    mProgram->setUniformValue("material.Kd", 0.8f, 0.8f, 0.8f);
    mProgram->setUniformValue("light.Ld", 1.0f, 1.0f, 1.0f);
    mProgram->setUniformValue("light.Position", mCamera.toMatrix() * lightPosition);
    mProgram->setUniformValue("material.Ka", 0.5f, 0.5f, 0.5f);
    mProgram->setUniformValue("light.La", 0.7f, 0.7f, 0.7f);
    mProgram->setUniformValue("material.Ks", 0.4f, 0.4f, 0.4f);
    mProgram->setUniformValue("light.Ls", 1.0f, 1.0f, 1.0f);
    mProgram->setUniformValue("material.Shininess", 50.0f);

    mObject.bind();
    mProgram->setUniformValue("modelMat", mTransform.toMatrix());
    glDrawArrays(GL_TRIANGLES, 0, drawArraySize);
    mObject.release();

    mTexture->release();
    mDisplacement->release();
    mVertex.release();
    mProgram->release();
}

Vector normalize(Vector v) {
    float mag2 = v.x() * v.x() + v.y() * v.y() + v.z() * v.z();
    if (mag2 > 0) {
        float invMag = 1 / sqrtf(mag2);
        return Vector(v.x() * invMag, v.y() * invMag, v.z() * invMag);
    }
    return v;
};

#define MAX_RAY_TRACING_DEPTH 5
QColor RenderingWidget::trace(const Ray ray, int depth, Light light) {
    if (depth > MAX_RAY_TRACING_DEPTH)
        return Qt::black;
    
    if (pScene == nullptr)
        return Qt::black;

    if (ray.is_degenerate())
        return Qt::black;

    QColor result;

    float minDist = 1e10;
    
    object *hitObject = nullptr;
    TreeandTri *hitT;
    Point hitCoord;
    Vector hitNormal;
    int hitFaceId = -1;
    
    Point rayStart = ray.start();
    Vector rayDir = normalize(ray.to_vector());

    auto squareDistance = [](Point a, Point b) {
        return (a.x() - b.x()) * (a.x() - b.x())
            + (a.y() - b.y()) * (a.y() - b.y())
            + (a.z() - b.z()) * (a.z() - b.z());
    };

    for (int i = 0; i < pScene->aabbTrees.size(); i++) {
        auto t = pScene->aabbTrees[i];
        if (t->tree.do_intersect(ray)) {
            Ray_intersection intersec = t->tree.first_intersection(ray);
            Point *pointIntersec = boost::get<Point>(&(intersec->first));
            int faceId = std::distance(t->triangles.begin(), intersec->second);
            float dist = squareDistance(rayStart, *pointIntersec);
            if (dist < minDist) {
                minDist = dist;
                hitT = t;
                hitCoord = *(pointIntersec);
                hitFaceId = faceId;
                hitObject = pScene->objects[i];
            }
        }
    }

    // No intersection
    if (hitFaceId == -1) {
        return Qt::black;
    }

    auto calcNormal = [] (Triangle t) {
        Vector v2v1(t[1], t[0]), v2v3(t[1], t[2]);
        Vector result = CGAL::cross_product(v2v3, v2v1);
        return CGAL::cross_product(v2v3, v2v1);
    };

    hitNormal = normalize(calcNormal(hitT->triangles[hitFaceId]));
    
    float bias = 1e-4;
    
    auto reflect = [](Vector I, Vector N) {
        return I - 2 * (I * N) * N;
    };
    auto clamp = [](float lo, float hi, float n) {
        return std::max(lo, std::min(hi, n));
    };
    auto refract = [&](Vector I, Vector N, float ior) {
        float cosi = clamp(-1, 1, (I * N));
        float etai = 1, etat = ior;
        Vector n = N;
        if (cosi < 0) { 
            cosi = -cosi; 
        }
        else { 
            std::swap(etai, etat); 
            n = -N; 
        }
        
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        
        if (k < 0) {
            return Vector(0, 0, 0);
        }
        else {
            return eta * I + (eta * cosi - sqrtf(k)) * n;
        }
       
    };
    auto fresnel = [&] (Vector I, Vector N, const float &ior, float &kr)
    {
        float cosi = clamp(-1, 1, (I * N));
        float etai = 1, etat = ior;
        if (cosi > 0) { std::swap(etai, etat); }
        // Compute sini using Snell's law
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            kr = 1;
        }
        else {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
        // As a consequence of the conservation of energy, transmittance is given by:
        // kt = 1 - kr;
    };

    switch (hitObject->material.Type) {
    case REFLECTION_AND_REFRACTION: {
        Vector reflectDir = normalize(reflect(rayDir, hitNormal));
        Vector refractDir = normalize(refract(rayDir, hitNormal, hitObject->material.ior));
        Point reflectCoord = (reflectDir * hitNormal) < 0 ? 
            hitCoord - hitNormal * bias : 
            hitCoord + hitNormal * bias;
        Point refractCoord = (refractDir * hitNormal) < 0 ? 
            hitCoord - hitNormal * bias :
            hitCoord + hitNormal * bias;
        QColor reflectColor = trace(Ray(reflectCoord, reflectDir), depth + 1, light);
        QColor refractionColor = trace(Ray(refractCoord, refractDir), depth + 1, light);
        
        float kr;
        fresnel(rayDir, hitNormal, hitObject->material.ior, kr);
        
        float resultRed = reflectColor.redF() * kr + refractionColor.redF() * (1 - kr);
        float resultGreen = reflectColor.greenF() * kr + refractionColor.greenF() * (1 - kr);
        float resultBlue = reflectColor.blueF() * kr + refractionColor.blueF() * (1 - kr);
        result.setRedF(resultRed);
        result.setGreenF(resultGreen);
        result.setBlueF(resultBlue);
        break;
    }
    case REFLECTION: {
        float kr;
        fresnel(rayDir, hitNormal, hitObject->material.ior, kr);
        Vector reflectDir = normalize(reflect(rayDir, hitNormal));
        Point reflectCoord = (reflectDir * hitNormal) < 0 ?
            hitCoord - hitNormal * bias :
            hitCoord + hitNormal * bias;
        QColor reflectColor = trace(Ray(reflectCoord, reflectDir), depth + 1, light);
        float resultRed = reflectColor.redF();
        float resultGreen = reflectColor.greenF();
        float resultBlue = reflectColor.blueF();
        result.setRedF(resultRed);
        result.setGreenF(resultGreen);
        result.setBlueF(resultBlue);
        break;
    }
    default: {
        Vector lightAmt(0, 0, 0), specularColor(0, 0, 0);
        Point shadowCoord = (rayDir * hitNormal) < 0 ?
            hitCoord + hitNormal * bias :
            hitCoord - hitNormal * bias;
        
        Point lightCoord(light.Position.x(), light.Position.y(), light.Position.z());
        Vector lightDir = lightCoord - hitCoord;
        // square of the distance between hitPoint and the light
        float lightSquareDistance = lightDir * lightDir;
        lightDir = normalize(lightDir);
        float LdotN = std::max(0.f, static_cast<float>(lightDir * hitNormal));
        
        object *shadowHitObject = nullptr;
        bool inShadow = false;
        Ray shadow(shadowCoord, lightDir);

        // is the point in shadow, and is the nearest occluding object closer to the object than the light itself?
        for (int i = 0; i < pScene->aabbTrees.size(); i++) {
            auto t = pScene->aabbTrees[i];
            if (t->tree.do_intersect(shadow)) {
                Ray_intersection intersec = t->tree.first_intersection(shadow);
                Point *pointIntersec = boost::get<Point>(&(intersec->first));
                int faceId = std::distance(t->triangles.begin(), intersec->second);
                float dist = squareDistance(shadowCoord, *pointIntersec);
                if (dist < lightSquareDistance) {
                    inShadow = true;
                }
            }
        }
        
        Vector lightIntensity(light.La, light.La, light.La);
        lightAmt += (1 - inShadow) * lightIntensity * LdotN;
        Vector reflectDir = normalize(reflect(-lightDir, hitNormal));
        specularColor += powf(std::max(0.f, static_cast<float>(-(reflectDir * rayDir))), hitObject->material.Shininess) * lightIntensity;
        
        
        Vector diffColor(lightAmt.x() * hitObject->material.diffColor.x(),
            lightAmt.y() * hitObject->material.diffColor.y(),
            lightAmt.z() * hitObject->material.diffColor.z());
        Vector res = diffColor* hitObject->material.Kd + specularColor * hitObject->material.Ks;
        
        result.setRedF(res.x());
        result.setGreenF(res.y());
        result.setBlueF(res.z());
        break;
    }
    }
    return result;
}

void RenderingWidget::renderObjectRayTracing(Light light) {
    int imageWidth = this->width(), imageHeight = this->height();
    QImage result(imageWidth, imageHeight, QImage::Format_ARGB32);

    Camera3D camera = mCamera;
    float fovVertical = 135.f;

    auto calcPrimaryRay = [&](int x, int y) {
        // (0, 0) is left top
        float nearPlane = 0.1f;
        QMatrix4x4 camMat = camera.toMatrix().inverted();
        QVector3D cam(camMat(0, 3), camMat(1, 3), camMat(2, 3));
        
        QVector3D camForward = camera.forward();
        camForward.normalize();
        camForward *= nearPlane;
        QVector3D camRight = camera.right(), camUp = camera.up();
        camRight.normalize();
        camUp.normalize();
        float pixelSize = (2 * nearPlane * tan(fovVertical / 360) / imageHeight);
        camRight *= pixelSize;
        camUp *= pixelSize;

        int centerX = imageWidth / 2, centerY = imageHeight / 2;

        camRight *= (x - centerX);
        camUp *= (centerY - y);

        QVector3D pixel = cam + camForward + camRight + camUp;
        
        Point camP(cam.x(), cam.y(), cam.z());
        Point pixelP(pixel.x(), pixel.y(), pixel.z());
        
        return Ray(camP, pixelP);
    };

    for (int x = 0; x < imageWidth; x++) {
        for (int y = 0; y < imageHeight; y++) {
            Ray prim = calcPrimaryRay(x, y);
            result.setPixelColor(x, y, trace(prim, 0, light));
        }
    }

    result.save("rt.jpg", "JPG");
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