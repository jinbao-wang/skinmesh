#ifndef SKELETONORIENTATION_H
#define SKELETONORIENTATION_H
#include "utils.h"

class orientation
{
public:

    orientation();
    ~orientation(){}

    QMap<QString, QMatrix4x4> calcRotationByJoints(QMap<QString, QVector3D> bone);

    void run();

    bool isFaceFront = true;

private:

    QMatrix4x4 calcRotation(QVector3D va1, QVector3D va2);
    QMatrix4x4 calcRotation(QVector3D x);
    QVector3D calcPointInOpenGLCoords(QVector3D vec, QString position);
    QVector3D calcSubjointInJointCoords(QMatrix3x3 jointRotation, QVector3D vJoint, QVector3D vSubjoint);
    QMatrix4x4 calcRotationAndAvoidSpinning(QVector3D v_joint);
    float calcAngleTwoVector(QVector3D va, QVector3D vb);
    float calcRootTurnAroundRotateAngle(QVector3D va, QVector3D vb);

    struct plane
    {
        float a, b, c, d;
        enum PLANE_STATES{
            PLANE_FRONT,
            PLANE_BACK,
            PLANE_ON
        };
        void createPlane(QVector3D v1, QVector3D v2, QVector3D v3);
        float distance(QVector3D v4);
        PLANE_STATES classifyPoint(QVector3D v4, float sigma);
        QVector3D normal(QVector3D v1, QVector3D v2, QVector3D v3);
        double calcAngleTwoPlane(QVector3D a, QVector3D b);
    };





};

#endif // SKELETONORIENTATION_H
