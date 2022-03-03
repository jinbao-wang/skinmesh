QT += core gui

TARGET = skinMesh
TEMPLATE = app


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -LD:\assimp-3.3.1\lib -lassimp \

INCLUDEPATH += D:\assimp-3.3.1\include \
               D:\eigen-3.3.6 \


#release:DEFINES += QT_NO_WARNING_OUTPUT\
#                   QT_NO_DEBUG_OUTPUT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

FORMS += \
    mainwindow.ui

HEADERS += \
    camera.h \
    glwidget.h \
    mainwindow.h \
    orientation.h \
    skinnedmesh.h \
    utils.h \
    configuration.h \



SOURCES += main.cpp \
    camera.cpp \
    glwidget.cpp \
    mainwindow.cpp \
    orientation.cpp \
    skinnedmesh.cpp \
    utils.cpp \


DISTFILES += \
    base.frag \
    base.vert \ \
    floor.frag \
    floor.vert
