#include "utils.h"



QMatrix4x4 getQmatrix4x4(aiMatrix4x4 ai)
{
    return QMatrix4x4( ai.a1,ai.a2,ai.a3,ai.a4,
                       ai.b1,ai.b2,ai.b3,ai.b4,
                       ai.c1,ai.c2,ai.c3,ai.c4,
                       ai.d1,ai.d2,ai.d3,ai.d4);
}


QMatrix4x4 getQmatrix4x4(QMatrix3x3 ai)
{
    return QMatrix4x4( ai(0,0),ai(0,1),ai(0,2),0,
                       ai(1,0),ai(1,1),ai(1,2),0,
                       ai(2,0),ai(2,1),ai(2,2),0,
                       0,      0,      0,      1);
}


QMatrix3x3 getQmatrix3x3(QMatrix4x4 ai)
{
    QMatrix3x3 a;
    a(0,0)=ai(0,0);a(0,1)=ai(0,1);a(0,2)=ai(0,2);
    a(1,0)=ai(1,0);a(1,1)=ai(1,1);a(1,2)=ai(1,2);
    a(2,0)=ai(2,0);a(2,1)=ai(2,1);a(2,2)=ai(2,2);

    return a;
}

QMatrix3x3 getRightHandCoords(QMatrix3x3 joint)
{
    QQuaternion qua=QQuaternion::fromRotationMatrix(joint);

    QQuaternion tmp;
    tmp.setScalar(qua.scalar());
    tmp.setX(-qua.x());
    tmp.setY(qua.y());
    tmp.setZ(qua.z());

    return tmp.toRotationMatrix();
}


QVector3D getAngleAxis(QMatrix4x4 m)
{
    Eigen::Matrix3d a;
    Eigen::AngleAxisd c;

    a<<m(0,0),m(0,1),m(0,2),
       m(1,0),m(1,1),m(1,2),
       m(2,0),m(2,1),m(2,2);
    c=a;
    QVector3D angle(c.axis()[0]*360,c.axis()[1]*360,c.axis()[2]*360);

    return angle;
}


QMatrix3x3 getQmatrix3x3(QMatrix3x3 ai)
{
    QMatrix3x3 a;
    a(0,0)=ai(0,0);a(0,1)=ai(0,1);a(0,2)=ai(0,2);
    a(1,0)=ai(1,0);a(1,1)=ai(1,1);a(1,2)=ai(1,2);
    a(2,0)=ai(2,0);a(2,1)=ai(2,1);a(2,2)=ai(2,2);

    return a;
}

QMatrix3x3 getQmatrix3x3(QVector3D a, QVector3D b, QVector3D c)
{
    QMatrix3x3 vec;
    vec(0,0)=a.x();vec(0,1)=b.x();vec(0,2)=c.x();
    vec(1,0)=a.y();vec(1,1)=b.y();vec(1,2)=c.y();
    vec(2,0)=a.z();vec(2,1)=b.z();vec(2,2)=c.z();

    return vec;
}


QVector3D getQVector3D(QMatrix3x3 rotation, QVector3D va)
{
    QVector3D vector;
    vector.setX(rotation(0,0) * va.x() + rotation(0,1) * va.y() + rotation(0,2) * va.z());
    vector.setY(rotation(1,0) * va.x() + rotation(1,1) * va.y() + rotation(1,2) * va.z());
    vector.setZ(rotation(2,0) * va.x() + rotation(2,1) * va.y() + rotation(2,2) * va.z());

    return vector;
}

float getAngleTwoVector(QVector3D va, QVector3D vb)
{
    float dotProduct = QVector3D::dotProduct(va, vb);
    float angle = qAcos(dotProduct / va.length() / vb.length()) / M_PI * 180;

    return angle;
}

QMatrix4x4 getQmatrix4x4(QMatrix3x3 ai, QVector3D va)
{
    return QMatrix4x4( ai(0,0),ai(0,1),ai(0,2),va.x(),
                       ai(1,0),ai(1,1),ai(1,2),va.y(),
                       ai(2,0),ai(2,1),ai(2,2),va.z(),
                       0,      0,      0,      1);

}

QVector3D getQVector3D(QMatrix4x4 ai, QVector3D va1)
{
    QVector3D vector;

    vector.setX(ai(0,0) * va1.x() + ai(0,1) * va1.y() + ai(0,2) * va1.z() + ai(0,3));
    vector.setY(ai(1,0) * va1.x() + ai(1,1) * va1.y() + ai(1,2) * va1.z() + ai(1,3));
    vector.setZ(ai(2,0) * va1.x() + ai(2,1) * va1.y() + ai(2,2) * va1.z() + ai(2,3));

    return vector;
}


QMatrix3x3 getQmatrix3x3(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8, float v9)
{
    QMatrix3x3 mat;
    mat(0,0)=v1; mat(0,1)=v2; mat(0,2)=v3;
    mat(1,0)=v4; mat(1,1)=v5; mat(1,2)=v6;
    mat(2,0)=v7; mat(2,1)=v8; mat(2,2)=v9;

    return mat;
}
