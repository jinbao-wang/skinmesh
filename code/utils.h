#ifndef UTILS_H
#define UTILS_H

#include "assimp/matrix4x4.h"
#include <QMatrix4x4>
#include <QMatrix3x3>
#include <Eigen/Dense>
#include "qmath.h"


QMatrix3x3 getQmatrix3x3(QMatrix4x4 ai);
QMatrix3x3 getQmatrix3x3(QMatrix3x3 ai);
QMatrix3x3 getQmatrix3x3(QVector3D a, QVector3D b, QVector3D c);

QMatrix3x3 getQmatrix3x3(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8, float v9);

QMatrix4x4 getQmatrix4x4(aiMatrix4x4 ai);
QMatrix4x4 getQmatrix4x4(QMatrix3x3 ai);

QMatrix4x4 getQmatrix4x4(QMatrix3x3 ai, QVector3D va);

QVector3D getQVector3D(QMatrix4x4 ai, QVector3D va);
QVector3D getQVector3D(QMatrix3x3 rotation, QVector3D va);


QVector3D getAngleAxis(QMatrix4x4 m);
float getAngleTwoVector(QVector3D a, QVector3D b);

QMatrix3x3 getRightHandCoords(QMatrix3x3 joint);



#endif // UTILS_H
