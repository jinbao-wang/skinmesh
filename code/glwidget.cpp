#include "glwidget.h"
#include <iostream>


GLWidget::GLWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    m_camera(0.0f, 0.0f, 40.0f, 0.0f, 0.0f, 0.0f),
    m_camera_init(0.0f, 0.0f, 40.0f, 0.0f, 0.0f, 0.0f),
    m_timer(nullptr),
    skinmesh(nullptr),
    mode(GL_TRIANGLES),
    m_program(nullptr),
    m_texture(nullptr),
    m_matrixUniform(0),
    verShader(nullptr),
    frgShader(nullptr)
{}

GLWidget::~GLWidget() {
    makeCurrent();

    delete skinmesh;
    delete m_program;
    m_vbo.destroy();

    doneCurrent();
}

void GLWidget::initializeGL() {
    initializeOpenGLFunctions();

    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex,   QString::fromStdString(config.VertexPath));
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, QString::fromStdString(config.FragmentPath));
    m_program->link();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start(1000 / 60.0f);


    for (uint i = 0; i < Maxbones; i++)
    {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        snprintf(Name, sizeof(Name), "gBones[%d]", i);
        m_bonelocation[i] = glGetUniformLocation(1, Name);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    verShader = new QOpenGLShader(QOpenGLShader::Vertex);
    frgShader = new QOpenGLShader(QOpenGLShader::Fragment);

    verShader->compileSourceFile(QString::fromStdString(config.FloorVertexPath));
    frgShader->compileSourceFile(QString::fromStdString(config.FloorFragmentPath));
}

void GLWidget::resizeGL(int w, int h) {
    m_projectionMat.setToIdentity();
    m_projectionMat.perspective(45.0f, w / float(h), 1.0f, 1000.0f);
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (isDetected) {
        float x = qBound(-100.0f, m_camera_init.position().x() + displacement.x() / 70.0f, 100.0f);
        float z = qBound(30.0f, m_camera_init.position().z() + displacement.z() / 180.0f - 0.f, 600.0f);

        m_camera.setTargetX(-x);
        m_camera.setPositionZ(z);
    }

    QMatrix4x4 viewMat   = m_camera.view();
    QMatrix4x4 modelview = viewMat * m_modelMat;
    m_normalMat = modelview.normalMatrix();

    // m_normalMat=m_modelMat.inverted().transposed();

    m_program->bind();
    m_program->setUniformValue("model",      m_modelMat);
    m_program->setUniformValue("view",       viewMat);
    m_program->setUniformValue("projection", m_projectionMat);
    m_program->setUniformValue("normalmat",  m_normalMat);


    if (skinmesh != 0)
    {
        QVector<QMatrix4x4> Transforms;
        skinmesh->BoneTransform(Transforms);
        setBoneTranforms(Transforms);
        skinmesh->render(this, mode);
    }
}

void GLWidget::loadModel(QString filename) {
    makeCurrent();
    m_camera_init = Camera(0.0f, 8.0f, 40.0f, 0.0f, 8.0f, 0.0f);
    m_camera      = Camera(0.0f, 8.0f, 40.0f, 0.0f, 8.0f, 0.0f);

    if (skinmesh != 0) {
        delete skinmesh;
    }
    skinmesh = new SkinnedMesh;

    if (!skinmesh->loadMesh(filename, this))
    {
        std::cout << "load model failed";
    }
    doneCurrent();
}

void GLWidget::wheelEvent(QWheelEvent *event) {
    float z = m_camera.position().z();

    z = qBound(0.0f, z - event->angleDelta().y() / 100.0f, 600.0f);

    m_camera.setPositionZ(z);
    m_camera_init.setPositionZ(z);
    event->accept();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    Qt::MouseButtons buttons = event->buttons();

    if (buttons & Qt::LeftButton) {
        if (m_lastMousePos.isNull()) {
            m_lastMousePos = event->pos();
        }
        else {
            QPoint offset = event->pos() - m_lastMousePos;
            m_modelMat.rotate((float)offset.x(), 0.0f, 1.0f, 0.0f);
            m_lastMousePos = event->pos();
        }
    }
    else if (buttons & Qt::RightButton) {
        if (m_lastMousePos.isNull()) {
            m_lastMousePos = event->pos();
        }
        else {
            QPoint offset = event->pos() - m_lastMousePos;
            m_camera.setPosition(m_camera.position().x() - offset.x() / 50.0f, m_camera.position().y() + offset.y() / 50.0f, m_camera.position().z());
            m_camera.setTarget(m_camera.target().x() - offset.x() / 50.0f, m_camera.target().y() + offset.y() / 50.0f, m_camera.target().z());

            // m_camera.setPositionY(m_camera.position().y() + offset.y() /
            // 20.0f);
            // m_camera.setTargetY(m_camera.target().y() + offset.y() / 20.0f);
            m_lastMousePos = event->pos();
        }
    }
    event->accept();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    m_lastMousePos = QPoint();
    event->accept();
}

void GLWidget::setPolygonMode(bool wireframe) {
    makeCurrent();

    if (wireframe) {
        mode = GL_LINE_LOOP;

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        mode = GL_TRIANGLES;

        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    doneCurrent();
}

void GLWidget::setBoneTranforms(QVector<QMatrix4x4>Transforms)
{
    for (uint i = 0; i < Transforms.size(); i++)
    {
        setBoneTranform(i, Transforms[i]);
    }
}

QMap<QString, QMatrix4x4>GLWidget::getNodeTransformationTPoseM()
{
    return skinmesh->NodeTransformationTPoseMatrix;
}

void GLWidget::setBoneTranform(uint index, const QMatrix4x4& Transform)
{
    assert(index < Maxbones);
    m_program->setUniformValue(m_bonelocation[index], Transform);

    // glUniformMatrix4fv(m_bonelocation[index],1,GL_TRUE,(const
    // GLfloat*)Transform);
}

void GLWidget::makeBoneTransforms(QMap<QString, QMatrix4x4>bodySkeletonMap)
{
    makeCurrent();

    if (skinmesh)
    {
        skinmesh->makeBoneTranformHeirarchySetAll(bodySkeletonMap);
    }
    doneCurrent();
}
