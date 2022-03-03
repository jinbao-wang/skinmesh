#include <QDebug>
#include <QApplication>
#include <QSurfaceFormat>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QSurfaceFormat format;

    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);


    QApplication a(argc, argv);
    MainWindow   w;


    w.initialize();


    w.show();

    return a.exec();
}
