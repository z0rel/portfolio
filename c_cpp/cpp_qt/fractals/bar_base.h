#ifndef BAR_BASE_H
#define BAR_BASE_H

#include <QVector>

#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include <qwt3d_plot3d.h>


using namespace Qwt3D;


class MainWindowBase;


/**
 * @brief Класс Bar для рисований
 * Класс, реализующий функциональность рисования фрактала.
 * Предоставляет интерфейс рисования на другом оконном объекте.
 */
class BarBase : public Qwt3D::VertexEnrichment {
public:
    /// Тип функций афинных преобразований
    typedef void (*Tfunc)(double x_res[2], double x[2]);

    BarBase(MainWindowBase *_parent)
        : parent(_parent) {}

    void drawBegin();
    void draw(Qwt3D::Triple const&);

    /// Рисовать точками а не линиями
    bool isPoints = false;
    /// Не рисовать. Нужно для ускорения работы при расчете масштаба
    bool noPaint = true;

protected:
    /// Родительское окно, нужно для вызова функции изменения масштаба.
    MainWindowBase *parent;

    /// Компонента цвета фрактала - Red
    int globR = 0;
    /// Компонента цвета фрактала - Green
    int globG = 0;
    /// Компонента цвета фрактала - Blue
    int globB = 255;

    /**
     * Глобальный массив вершин (vertex array) для буферизованного вывода OpenGl.
     * Является компонентом агрессивной оптимизации.
     */
    GLfloat pVerts[1024 * 16];

    /// Указатель текущего элемента в pVerts
    int vertCnt = 0;

    /// Текущая глубина отрисовки фрактала.
    int depth = 7;

    virtual void make_fractal() = 0;
    void paintVertexVector();
};

#endif // BAR_BASE_H
