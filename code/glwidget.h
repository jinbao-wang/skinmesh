#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QWheelEvent>
#include <QMouseEvent>
#include "skinnedmesh.h"
#include "camera.h"
#include "configuration.h"


#define Maxbones 100

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

public:

    GLWidget(QWidget *parent);
    ~GLWidget();

    void loadModel(QString filename);
    void makeBoneTransforms();
    void makeBoneTransforms(QString    Bonename,
                            QMatrix4x4 bodySkeleton);
    void makeBoneTransforms(QMap<QString, QMatrix4x4>bodySkeletonMap);

    bool isDetected = false;
    QVector3D displacement;

public slots:

    void                     setPolygonMode(bool wireframe);
    QMap<QString, QMatrix4x4>getNodeTransformationTPoseM();

protected:

    void initializeGL();
    void resizeGL(int w,
                  int h);
    void paintGL();
    void drawFloor();

    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void setBoneTranform(uint              index,
                         const QMatrix4x4& Transform);
    void setBoneTranforms(QVector<QMatrix4x4>Transforms);

private:

    QMatrix4x4 m_modelMat;
    QMatrix4x4 m_projectionMat;
    QMatrix3x3 m_normalMat;

    Camera m_camera;
    Camera m_camera_init;
    QTimer *m_timer;
    QPoint m_lastMousePos;
    SkinnedMesh *skinmesh;
    Configuration config;

    GLenum mode;

    QOpenGLShaderProgram *m_program;
    GLuint m_bonelocation[Maxbones];

    // draw floor
    QOpenGLTexture *m_texture;
    QOpenGLBuffer m_vbo;
    int m_matrixUniform;
    QOpenGLShader *verShader;
    QOpenGLShader *frgShader;
};

#endif // GLWIDGET_H
