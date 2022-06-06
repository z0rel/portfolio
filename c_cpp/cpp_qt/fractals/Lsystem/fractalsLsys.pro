QT       += core gui opengl widgets openglwidgets
TARGET   =  fractalsLsys
TEMPLATE =  app
CONFIG   += qt warn_on thread debug c++17

OBJECTS_DIR = ./tmpL
MOC_DIR     = ./tmpL
UI_DIR      = ./tmpL


SOURCES += \
    ../main.cpp \
    ../mainwindow_base.cpp \
    ../bar_base.cpp \
    bar_logic_lsystem.cpp \
    mainwindow_logic.cpp


HEADERS  += \
    ../hat.h \
    ../mainwindow_base.h \
    ../bar_base.h \
    bar_logic_lsystem.h \
    mainwindow_logic.h


FORMS += \
    mainwindow_fractals.ui


INCLUDEPATH += $$QWT3D_DIR/include ../
DEPENDPATH	= $$INCLUDEPATH


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
	 LIBS += -lGLC_lib -lqwtplot3d -L$$QWT3D_DIR/lib
	 INCLUDEPATH += "/usr/local/include/GLC_lib"
     QWT3D_DIR = /home/artem/developer/qwtplot3d/trunk/qwtplot3d
}



QMAKE_RESOURCE_FLAGS += -threshold 0 -compress 9

