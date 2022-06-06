QT += widgets openglwidgets
QT += core gui opengl
TARGET  =  fractalsAfin
TEMPLATE =  app
CONFIG += qt warn_on thread debug

OBJECTS_DIR = ./tmp
MOC_DIR     = ./tmp
UI_DIR      = ./tmp


CONFIG += c++17


SOURCES += main.cpp \
    mainwindow_base.cpp \
    bar_base.cpp \
    mainwindow_logic.cpp \
    bar_logic.cpp


HEADERS += \
    hat.h \
    mainwindow_base.h \
    bar_base.h \
    mainwindow_logic.h \
    bar_logic.h


FORMS += \
    mainwindow_fractals.ui


QMAKE_RESOURCE_FLAGS += -threshold 0 -compress 9


# git clone https://github.com/sintegrial/qwtplot3d

win32 {
    QWT3D_DIR = K:\developer\sandbox\qwtplot3d

    # qwt library defines
    CONFIG       += opengl
    MOC_DIR       = tmp
    OBJECTS_DIR   = tmp
    win32:CONFIG -= zlib

    INCLUDEPATH += $$QWT3D_DIR/include
    DEPENDPATH	= $$INCLUDEPATH $$QWT3D_DIR/src

    INCLUDEPATH += "$$(GLC_LIB_DIR)/include"

    SOURCES += $$QWT3D_DIR/src/*.cpp
    HEADERS += $$QWT3D_DIR/include/*.h
    LIBS += -lopengl32 -lglu32 -lglmf32
}

unix {
    QWT3D_DIR = /home/artem/developer/qwtplot3d/trunk/qwtplot3d
    INCLUDEPATH += $$QWT3D_DIR/include
    DEPENDPATH	= $$INCLUDEPATH

    LIBS += -lGLC_lib -lqwtplot3d -L$$QWT3D_DIR/lib
    INCLUDEPATH += "/usr/local/include/GLC_lib"
}


zlib {
    DEFINES += GL2PS_HAVE_ZLIB
    win32:LIBS += zlib.lib
    unix:LIBS  += -lz
}
