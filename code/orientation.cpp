#include "orientation.h"
#include <iostream>
#include <QDebug>
#include <QVector3D>


orientation::orientation() {}

// 计算任意两个向量的旋转矩阵
QMatrix4x4 orientation::calcRotation(QVector3D va1, QVector3D vb1)
{
    if (va1 == vb1.normalized()) {
        QMatrix4x4 Identity;
        return Identity;
    }

    QVector3D rotationAxis(1, 1, 1);

    // 参考坐标系，随意选取，不影响最后结果, 但是不能与前后旋转的向量一样，
    // 该向量决定了新的旋转是由该向量作为旋转轴在标准oepenGl坐标系旋转决定的

    va1 = va1.normalized();
    QVector3D va2 = QVector3D::crossProduct(va1, rotationAxis);
    va2 = va2.normalized();
    QVector3D va3 = QVector3D::crossProduct(va2, va1);
    va3 = va3.normalized();
    QMatrix3x3 veca = getQmatrix3x3(va1, va2, va3);

    vb1 = vb1.normalized();
    QVector3D vb2 = QVector3D::crossProduct(vb1, rotationAxis);
    vb2 = vb2.normalized();
    QVector3D vb3 = QVector3D::crossProduct(vb2, vb1);
    vb3 = vb3.normalized();
    QMatrix3x3 vecb = getQmatrix3x3(vb1, vb2, vb3);

    QMatrix3x3 rotation = vecb * veca.transposed();

    return getQmatrix4x4(rotation);
}

// 计算某个向量由x轴旋转过来的旋转矩阵，使得每次新向量方向为新x轴指向的位置
QMatrix4x4 orientation::calcRotation(QVector3D x)
{
    QMatrix4x4 Identity;

    Identity.setToIdentity();

    // 如果骨骼本身处在x轴方向上，返回单位矩阵
    // 如果骨骼朝向与x轴反向，则返回将单位阵绕y轴或z轴旋转180度后的矩阵
    if (QVector3D(1, 0, 0) == x.normalized()) {
        return Identity;
    }

    if (QVector3D(-1, 0, 0) == x.normalized()) {
        Identity.rotate(180, QVector3D(0, 0, 1));
        return Identity;
    }

    x = x.normalized();
    QVector3D y = QVector3D::crossProduct(QVector3D(1, 0, 0), x);
    y = y.normalized();
    QVector3D z = QVector3D::crossProduct(x, y);
    z = z.normalized();

    QMatrix3x3 matrix = getQmatrix3x3(x, y, z);

    return getQmatrix4x4(matrix);
}

// 计算输入的点局部坐标系下的位置坐标
QVector3D orientation::calcPointInOpenGLCoords(QVector3D vec, QString position)
{
    if (position == "left") {
        return vec;
    }

    if (position == "right") {
        return QVector3D(-vec.x(), -vec.y(), -vec.z());
    }

    if (position == "up") {
        return QVector3D(vec.y(), -vec.x(), -vec.z());
    }

    if (position == "down") {
        return QVector3D(-vec.y(), vec.x(), -vec.z());
    }

    return vec;
}

// 计算子关节点在父关节点坐标系中的位置
QVector3D orientation::calcSubjointInJointCoords(QMatrix3x3 jointRotation, QVector3D vJoint, QVector3D vSubjoint)
{
    // rotation: 以(0,0,0)为原点的vJoint的新坐标系，与OpenGL坐标系原点重合
    // translation: vJoint在新坐标系下的位置，作为新坐标系的平移量，即以下一个子关节点为原点
    // transform: 计算vSubjoint在新坐标系下的旋转矩阵
    // vSubjointNew: 子关节点在父关节点为原点坐标系中的位置

    QMatrix3x3 rotation     = jointRotation;
    QVector3D  translation  = getQVector3D(rotation.transposed(), vJoint);
    QMatrix4x4 transform    = getQmatrix4x4(rotation.transposed(), -translation);
    QVector3D  vSubjointNew = getQVector3D(transform, vSubjoint);

    return vSubjointNew;
}

// 通过关节点，计算每个骨骼的旋转矩阵
QMap<QString, QMatrix4x4>orientation::calcRotationByJoints(QMap<QString, QVector3D>joint)
{
    // calculate the bone vector
    QVector3D v_root, v_root_x;
    QVector3D v_head;
    QVector3D v_l_shoulder, v_l_elbow, v_l_hip, v_l_knee; // v_l_hand, v_r_hand
    QVector3D v_r_shoulder, v_r_elbow, v_r_hip, v_r_knee; // v_l_foot, v_r_foot

    v_root       = (joint["r_shoulder"] + joint["l_shoulder"]) / 2 - joint["root"];
    v_root_x     = joint["l_shoulder"] - joint["r_shoulder"];
    v_head       = joint["head"] - joint["root"];
    v_l_shoulder = joint["l_elbow"] - joint["l_shoulder"];
    v_l_elbow    = joint["l_hand"] - joint["l_shoulder"];
    v_r_shoulder = joint["r_elbow"] - joint["r_shoulder"];
    v_r_elbow    = joint["r_hand"] - joint["r_shoulder"];
    v_l_hip      = joint["l_knee"] - joint["l_hip"];
    v_l_knee     = joint["l_foot"] - joint["l_hip"];
    v_r_hip      = joint["r_knee"] - joint["r_hip"];
    v_r_knee     = joint["r_foot"] - joint["r_hip"];

    // calculate the local rotation matrix for each bone
    QMatrix4x4 root, head, Identity;
    QMatrix4x4 l_shoulder, l_elbow, l_hip, l_knee;
    QMatrix4x4 r_shoulder, r_elbow, r_hip, r_knee;

    QMatrix4x4 up, down;

    // set the bone orientation pointing into the new x-axis
    up.rotate(-90, QVector3D(0, 0, 1));
    down.rotate(90, QVector3D(0, 0, 1));

    // change the openGL coordinates into local coordinates
    v_root       = calcPointInOpenGLCoords(v_root, "up");
    v_head       = calcPointInOpenGLCoords(v_head, "up");
    v_l_shoulder = calcPointInOpenGLCoords(v_l_shoulder, "left");
    v_l_elbow    = calcPointInOpenGLCoords(v_l_elbow, "left");
    v_r_shoulder = calcPointInOpenGLCoords(v_r_shoulder, "right");
    v_r_elbow    = calcPointInOpenGLCoords(v_r_elbow, "right");
    v_l_hip      = calcPointInOpenGLCoords(v_l_hip, "down");
    v_l_knee     = calcPointInOpenGLCoords(v_l_knee, "down");
    v_r_hip      = calcPointInOpenGLCoords(v_r_hip, "down");
    v_r_knee     = calcPointInOpenGLCoords(v_r_knee, "down");

    // calculate the bone rotation matrix and avoid the new x-axis spinning
    // itself
    root       = calcRotationAndAvoidSpinning(v_root);
    head       = calcRotationAndAvoidSpinning(v_head);
    l_shoulder = calcRotationAndAvoidSpinning(v_l_shoulder);
    l_elbow    = calcRotationAndAvoidSpinning(calcSubjointInJointCoords(l_shoulder.normalMatrix(), v_l_shoulder, v_l_elbow));
    r_shoulder = calcRotationAndAvoidSpinning(v_r_shoulder);
    r_elbow    = calcRotationAndAvoidSpinning(calcSubjointInJointCoords(r_shoulder.normalMatrix(), v_r_shoulder, v_r_elbow));
    l_hip      = calcRotationAndAvoidSpinning(v_l_hip);
    l_knee     = calcRotationAndAvoidSpinning(calcSubjointInJointCoords(l_hip.normalMatrix(), v_l_hip, v_l_knee));
    r_hip      = calcRotationAndAvoidSpinning(v_r_hip);
    r_knee     = calcRotationAndAvoidSpinning(calcSubjointInJointCoords(r_hip.normalMatrix(), v_r_hip, v_r_knee));

    // calculate the angle of body turn around
    float angle = calcRootTurnAroundRotateAngle(v_root_x, QVector3D(1, 0, 0));
    root.rotate(angle, QVector3D(1, 0, 0));

    if ((angle > 90) || (angle < -90)) {
        isFaceFront = !isFaceFront;
    }

    // make sure the display result is the same with the openGL space
    root   = up * root * up.inverted();
    head   = up * head * up.inverted();
    l_hip  = down * l_hip * down.inverted();
    l_knee = down * l_knee * down.inverted();
    r_hip  = down * r_hip * down.inverted();
    r_knee = down * r_knee * down.inverted();

    // incase rotation matrix is zeros when the bone vector is empty.
    if (v_root.isNull()) root = Identity;
    if (v_head.isNull()) head = Identity;
    if (v_l_shoulder.isNull()) l_shoulder = Identity;
    if (v_l_elbow.isNull()) l_elbow = Identity;
    if (v_r_shoulder.isNull()) l_shoulder = Identity;
    if (v_r_elbow.isNull()) r_elbow = Identity;
    if (v_l_hip.isNull()) l_hip = Identity;
    if (v_l_knee.isNull()) l_knee = Identity;
    if (v_r_hip.isNull()) r_hip = Identity;
    if (v_r_knee.isNull()) r_knee = Identity;

    // packaging the date
    QMap<QString, QMatrix4x4> rotationMap;
    rotationMap.insert(      "root", root);
    rotationMap.insert(      "head", head);
    rotationMap.insert("l_shoulder", l_shoulder);
    rotationMap.insert(   "l_elbow", l_elbow);
    rotationMap.insert("r_shoulder", r_shoulder);
    rotationMap.insert(   "r_elbow", r_elbow);
    rotationMap.insert(     "l_hip", l_hip);
    rotationMap.insert(    "l_knee", l_knee);
    rotationMap.insert(     "r_hip", r_hip);
    rotationMap.insert(    "r_knee", r_knee);

    return rotationMap;
}

void orientation::run()
{
    QVector3D v_l_shoulder(1, 1, 1);

    QMatrix4x4 rotation = calcRotationAndAvoidSpinning(v_l_shoulder);

    qDebug() << rotation;
}

// 计算骨骼旋转矩阵后，与OpenGL坐标系对比新坐标系的X轴会发生径向的旋转
// 本函数计算该旋转角度，返回修正后的旋转矩阵
QMatrix4x4 orientation::calcRotationAndAvoidSpinning(QVector3D v_joint)
{
    // 本方法采用迭代求解法获得新坐标系的自旋角度，迭代精度决定计算速度
    // 也可以直接计算y-o-x_reprojection平面与y_reprojection-o-x_reprojection的夹角，
    // 但是需要判断四种情况下哪个角度正确，未实现

    QVector3D o(0, 0, 0);  // 父关节坐标系原点
    QVector3D y(0, 1, 0);  // 父关节坐标系y轴
    QVector3D x_(1, 0, 0); // 子关节坐标系x轴
    QVector3D y_(0, 1, 0); // 子关节坐标系y轴

    QMatrix4x4 subSpaceRotation = calcRotation(v_joint);

    float spinAngle = 0;

    for (int i = -18000; i < 18000; i++)
    {
        spinAngle = i * 0.01;
        subSpaceRotation.rotate(spinAngle, x_);

        // 获得x_、y_在父关节空间的投影坐标位置
        QVector3D x_reprojection = getQVector3D(subSpaceRotation, x_);
        QVector3D y_reprojection = getQVector3D(subSpaceRotation, y_);
        plane     planer;
        planer.createPlane(o, y, x_reprojection);

        // 计算点y_reprojection到平面y-o-x_reprojection的距离
        plane::PLANE_STATES status = planer.classifyPoint(y_reprojection, 1e-2);

        // 计算内积，判断y_reprojection与y是否同向
        float innerProduct = QVector3D::dotProduct(y, y_reprojection);

        // 如果y_reprojection在y-o-x_reprojection平面内，且与y轴为锐角，则计算结束, 若为钝角，则为反向180度
        if ((plane::PLANE_ON == status) && (innerProduct >= 0)) break;
    }

    // qDebug()<<spinAngle;

    return subSpaceRotation;
}

float orientation::calcAngleTwoVector(QVector3D va, QVector3D vb)
{
    float dotProduct = QVector3D::dotProduct(va, vb);
    float angle      = qAcos(dotProduct / va.length() / vb.length()) / M_PI * 180.;

    return angle;
}

float orientation::calcRootTurnAroundRotateAngle(QVector3D va, QVector3D vb)
{
    float angle = calcAngleTwoVector(va, vb);

    if (va.z() < 0) {
        return -angle;
    }

    return angle;
}

// 由三个点确定一个平面
void orientation::plane::createPlane(QVector3D v1, QVector3D v2, QVector3D v3)
{
    QVector3D n = normal(v1, v2, v3);

    a = n.x();
    b = n.y();
    c = n.z();
    d = -(a * v1.x() + b * v1.y() + c * v1.z());
}

// 计算点到平面的距离
float orientation::plane::distance(QVector3D v4)
{
    return a * v4.x() + b * v4.y() + c * v4.z() + d;
}

// 判断点于平面的位置关系，分别为平面上-0、平面下-1、平面内-2
orientation::plane::PLANE_STATES orientation::plane::classifyPoint(QVector3D v4, float sigma)
{
    float dist = distance(v4);

    if (dist > sigma) {
        return PLANE_FRONT;
    }

    if (dist < -sigma) {
        return PLANE_BACK;
    }

    return PLANE_ON;
}

// 三个点求平面法向
QVector3D orientation::plane::normal(QVector3D v1, QVector3D v2, QVector3D v3)
{
    QVector3D n = QVector3D::crossProduct(v2 - v1, v3 - v2);

    n.normalize();

    return n;
}

// 计算两个平面的夹角
double orientation::plane::calcAngleTwoPlane(QVector3D a, QVector3D b)
{
    double k     = (a.x() * b.x() + a.y() * b.y() + a.z() * b.z()) / a.length() / b.length();
    double angle = qAcos(k) / M_PI * 180.;

    return angle;
}
