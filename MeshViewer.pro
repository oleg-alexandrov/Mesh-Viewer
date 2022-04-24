#-------------------------------------------------
#
# Project created by QtCreator 2017-05-18T20:06:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MeshViewer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    openglwindow.cpp \
    modelmanager.cpp

HEADERS  += mainwindow.h \
    openglwindow.h \
    modelmanager.h

FORMS    += mainwindow.ui

unix:!macx|win32: LIBS += -L/home/oalexan1/projects/assimp/build/bin -lassimp -lglut -lGL -lGLU

INCLUDEPATH += /home/oalexan1/projects/assimp/include
INCLUDEPATH += /home/oalexan1/projects/assimp/build/include
DEPENDPATH += /home/oalexan1/projects/assimp/include

