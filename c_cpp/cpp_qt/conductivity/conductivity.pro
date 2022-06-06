QT      += widgets openglwidgets
QT      += core gui opengl
TARGET   =  conductivity
TEMPLATE =  app
CONFIG   += qt warn_on thread release

OBJECTS_DIR = ./tmp
MOC_DIR     = ./tmp
UI_DIR      = ./tmp


CONFIG += c++17


DEPENDPATH	= $$INCLUDEPATH


SOURCES += \
    main.cpp \
    mainwindow_conductivity.cpp \
    glwidget.cpp \
    temperature.cpp \
    maketetraidersobj.cpp \
    bar.cpp


HEADERS += \
    mainwindow_conductivity.h \
    glwidget.h \
    global.h \
    temperature.h \
    maketetraidersobj.h \
    bar.h \
    hat.h


FORMS += \
    mainwindow_conductivity.ui


RESOURCES += \
    resources.qrc


QMAKE_RESOURCE_FLAGS += -threshold 0 -compress 9


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
	 LIBS += -lGLC_lib -lqwtplot3d -L./qwtplot3d/lib
	 INCLUDEPATH += "/usr/local/include/GLC_lib"
}

