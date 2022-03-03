#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "glwidget.h"
#include "orientation.h"
#include "configuration.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);

    void initialize();
    void loadAvatarModel();
    void bindAvatarBoneName();
    void bindBoneDataViaJoints();
    void riggingAvatarData();

    ~MainWindow();

public slots:

    void openFileDialog();
    void updateAvatarTransformData();
    void on_loadAvatar_clicked();
    void on_openCamera_clicked();
    void on_transformTest_clicked();

private:

    Ui::MainWindow *ui;
    GLWidget *view;
    QString lastFilePath;
    QTimer *timer;

    QMap<QString, QMatrix4x4>getNodeTransTPoseMatrix();
    QMap<QString, QMatrix4x4>getNodeTransMatrixByJoints(QMap<QString, QVector3D>Joints);
    QMatrix4x4               getNodeFinalMatrix(QMatrix4x4 NodeMatrix,
                                                QMatrix4x4 NodeTPoseM);

    // global variable starting with avatar
    // current frame data for updating ending with Tmp
    QString avatarName;
    QMap<QString, QString>avatarBoneName;
    QMap<QString, QVector3D>avatarBoneDataViaJointsTmp;
    QMap<QString, QMatrix4x4>avatarInterfaceDataTmp;

    orientation orientation;
    Configuration config;

    bool isLoadAvatar = false;
};

#endif // MAINWINDOW_H
