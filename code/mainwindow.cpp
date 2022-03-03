#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <Eigen/Dense>
#include <QDebug>
#include <QQuaternion>
#include <QVector3D>
#include <stdio.h>
#include <iostream>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    view = new GLWidget(ui->centralWidget);
    ui->centralLayout->insertWidget(0, view, 1);

    QAction *openAction = ui->fileMenu->addAction("Open Model...");
    connect(openAction,       &QAction::triggered, this, &MainWindow::openFileDialog);
    connect(ui->wireframeBox, &QCheckBox::toggled, view, &GLWidget::setPolygonMode);
}

MainWindow::~MainWindow() {
    delete ui;
    delete view;
    delete timer;
}

void MainWindow::openFileDialog() {
    QString directory = (lastFilePath.length() > 0) ? lastFilePath : QDir::homePath();
    QString filename  = QFileDialog::getOpenFileName(this,  "Open Model", directory, "models (*.dae *.obj *.fbx)");

    if (!filename.isNull()) {
        lastFilePath = filename;
        view->loadModel(filename);
        isLoadAvatar = true;
    }
}

void MainWindow::initialize()
{
    avatarName = QString::fromStdString(config.AvatarName);
    bindAvatarBoneName();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateAvatarTransformData()));

    timer->start(1000 / 60.0f);
}

// globally updating the data for animation
void MainWindow::updateAvatarTransformData()
{
    if (isLoadAvatar) {
        if (!avatarInterfaceDataTmp.isEmpty()) {
            view->makeBoneTransforms(avatarInterfaceDataTmp);
        }
    }
}

// button: Load Avatar
void MainWindow::on_loadAvatar_clicked()
{
    view->loadModel(QString::fromStdString(config.AvatarPath));
    isLoadAvatar = true;
}

void MainWindow::on_openCamera_clicked()
{
    std::cout << "empty" << std::endl;
}

// button: Transform, test movement, pipeline
void MainWindow::on_transformTest_clicked()
{
    bindBoneDataViaJoints();
    riggingAvatarData();
}

// calculate the bone transformation matrix form 3d coordinates
QMap<QString, QMatrix4x4>MainWindow::getNodeTransMatrixByJoints(QMap<QString, QVector3D>Joints)
{
    return orientation.calcRotationByJoints(Joints);
}

// obtain original transformation matrix of avatar model
QMap<QString, QMatrix4x4>MainWindow::getNodeTransTPoseMatrix()
{
    return view->getNodeTransformationTPoseM();
}

// coordinate transformation
QMatrix4x4 MainWindow::getNodeFinalMatrix(QMatrix4x4 NodeMatrix, QMatrix4x4 NodeTPoseM)
{
    NodeTPoseM(0, 3) = 0;
    NodeTPoseM(1, 3) = 0;
    NodeTPoseM(2, 3) = 0;
    NodeTPoseM(3, 3) = 1;

    return NodeTPoseM.inverted() * NodeMatrix * NodeTPoseM;
}

// make sure that the correspondance is right,
// or the bone rotation matrix will be set by Identity.
void MainWindow::bindAvatarBoneName()
{
    if (avatarName == "astroBoy") {
        avatarBoneName.insert("root",       "root");
        avatarBoneName.insert("head",       "neck01");
        avatarBoneName.insert("l_shoulder", "L_shoulder");
        avatarBoneName.insert("l_elbow",    "L_elbow");
        avatarBoneName.insert("r_shoulder", "R_shoulder");
        avatarBoneName.insert("r_elbow",    "R_elbow");
        avatarBoneName.insert("l_hip",      "L_hip");
        avatarBoneName.insert("l_knee",     "L_knee_02");
        avatarBoneName.insert("r_hip",      "R_hip");
        avatarBoneName.insert("r_knee",     "R_knee_02");
    } else if (avatarName == "girl") {
        avatarBoneName.insert("root",       "QuickRigCharacter2_Hips");
        avatarBoneName.insert("head",       "QuickRigCharacter2_Neck");
        avatarBoneName.insert("l_shoulder", "QuickRigCharacter2_LeftArm");
        avatarBoneName.insert("l_elbow",    "QuickRigCharacter2_LeftForeArm");
        avatarBoneName.insert("r_shoulder", "QuickRigCharacter2_RightArm");
        avatarBoneName.insert("r_elbow",    "QuickRigCharacter2_RightForeArm");
        avatarBoneName.insert("l_hip",      "QuickRigCharacter2_LeftUpLeg");
        avatarBoneName.insert("l_knee",     "QuickRigCharacter2_LeftLeg");
        avatarBoneName.insert("r_hip",      "QuickRigCharacter2_RightUpLeg");
        avatarBoneName.insert("r_knee",     "QuickRigCharacter2_RightLeg");
    } else {
        std::cout << "No this Avatar name, rigging filed!" << std::endl;
    }
}

void MainWindow::bindBoneDataViaJoints()
{
    // right hand coordinates
    // assign the 3d locations of joints for avatar
    QVector3D v_root       = QVector3D(0, 0, 0);
    QVector3D v_head       = QVector3D(0, 0, 0);
    QVector3D v_l_shoulder = QVector3D(0, 0, 0);
    QVector3D v_r_shoulder = QVector3D(0, 0, 0);
    QVector3D v_l_elbow    = QVector3D(1, 1, 1);
    QVector3D v_r_elbow    = QVector3D(-1, 0, 1);
    QVector3D v_l_hand     = QVector3D(2, 2, 2);
    QVector3D v_r_hand     = QVector3D(-2, 0, 2);
    QVector3D v_l_hip      = QVector3D(0, 0, 0);
    QVector3D v_r_hip      = QVector3D(0, 0, 0);
    QVector3D v_l_knee     = QVector3D(0, 0, 0);
    QVector3D v_r_knee     = QVector3D(-1, -1, -1);
    QVector3D v_l_foot     = QVector3D(0, 0, 0);
    QVector3D v_r_foot     = QVector3D(0, 0, 0);

    // global variable
    avatarBoneDataViaJointsTmp.insert("root",       v_root);
    avatarBoneDataViaJointsTmp.insert("head",       v_head);
    avatarBoneDataViaJointsTmp.insert("l_shoulder", v_l_shoulder);
    avatarBoneDataViaJointsTmp.insert("r_shoulder", v_r_shoulder);
    avatarBoneDataViaJointsTmp.insert("l_elbow",    v_l_elbow);
    avatarBoneDataViaJointsTmp.insert("r_elbow",    v_r_elbow);
    avatarBoneDataViaJointsTmp.insert("l_hand",     v_l_hand);
    avatarBoneDataViaJointsTmp.insert("r_hand",     v_r_hand);
    avatarBoneDataViaJointsTmp.insert("l_hip",      v_l_hip);
    avatarBoneDataViaJointsTmp.insert("r_hip",      v_r_hip);
    avatarBoneDataViaJointsTmp.insert("l_knee",     v_l_knee);
    avatarBoneDataViaJointsTmp.insert("r_knee",     v_r_knee);
    avatarBoneDataViaJointsTmp.insert("l_foot",     v_l_foot);
    avatarBoneDataViaJointsTmp.insert("r_foot",     v_r_foot);
}

void MainWindow::riggingAvatarData()
{
    QMap<QString, QMatrix4x4> Transformation  = getNodeTransMatrixByJoints(avatarBoneDataViaJointsTmp);
    QMap<QString, QMatrix4x4> NodeTPoseMatrix = getNodeTransTPoseMatrix();

    QMatrix4x4 root       = getNodeFinalMatrix(Transformation["root"],       NodeTPoseMatrix[avatarBoneName["root"]]);
    QMatrix4x4 head       = getNodeFinalMatrix(Transformation["head"],       NodeTPoseMatrix[avatarBoneName["head"]]);
    QMatrix4x4 l_shoulder = getNodeFinalMatrix(Transformation["l_shoulder"], NodeTPoseMatrix[avatarBoneName["l_shoulder"]]);
    QMatrix4x4 r_shoulder = getNodeFinalMatrix(Transformation["r_shoulder"], NodeTPoseMatrix[avatarBoneName["r_shoulder"]]);
    QMatrix4x4 l_elbow    = getNodeFinalMatrix(Transformation["l_elbow"],    NodeTPoseMatrix[avatarBoneName["l_elbow"]]);
    QMatrix4x4 r_elbow    = getNodeFinalMatrix(Transformation["r_elbow"],    NodeTPoseMatrix[avatarBoneName["r_elbow"]]);
    QMatrix4x4 l_hip      = getNodeFinalMatrix(Transformation["l_hip"],      NodeTPoseMatrix[avatarBoneName["l_hip"]]);
    QMatrix4x4 r_hip      = getNodeFinalMatrix(Transformation["r_hip"],      NodeTPoseMatrix[avatarBoneName["r_hip"]]);
    QMatrix4x4 l_knee     = getNodeFinalMatrix(Transformation["l_knee"],     NodeTPoseMatrix[avatarBoneName["l_knee"]]);
    QMatrix4x4 r_knee     = getNodeFinalMatrix(Transformation["r_knee"],     NodeTPoseMatrix[avatarBoneName["r_knee"]]);

    QMap<QString, QMatrix4x4> bodySkeletonMap;

    bodySkeletonMap.insert(avatarBoneName["root"],       root);
    bodySkeletonMap.insert(avatarBoneName["head"],       head);
    bodySkeletonMap.insert(avatarBoneName["l_shoulder"], l_shoulder);
    bodySkeletonMap.insert(avatarBoneName["l_elbow"],    l_elbow);
    bodySkeletonMap.insert(avatarBoneName["r_shoulder"], r_shoulder);
    bodySkeletonMap.insert(avatarBoneName["r_elbow"],    r_elbow);
    bodySkeletonMap.insert(avatarBoneName["l_hip"],      l_hip);
    bodySkeletonMap.insert(avatarBoneName["l_knee"],     l_knee);
    bodySkeletonMap.insert(avatarBoneName["r_hip"],      r_hip);
    bodySkeletonMap.insert(avatarBoneName["r_knee"],     r_knee);

    // global variable for updating
    avatarInterfaceDataTmp = bodySkeletonMap;
}
