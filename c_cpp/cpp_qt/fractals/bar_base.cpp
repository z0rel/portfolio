#include "bar_base.h"

#include "qbitmap.h"
#include "qwt3d_color.h"
#include "qwt3d_plot3d.h"
#include <iostream>
#include <iomanip>


using namespace std;


/// Обработчик инициализации перерисовки
void BarBase::drawBegin()
{
    vertCnt = 0;
    glLineWidth( 0 );
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1, 1);

    // Очистить экран.
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_SMOOTH);

    // Для ускорения движка отклчаем сглаживание фрактала
    glDisable(GL_LINE_SMOOTH);
}


/**
 * @brief paintVertexVector
 * Буферизованная отрисаовка. Накопленный буфер линий
 * "одним махом" выводится на экран.
 * Сделано, чтобы при большой глубине фрактала - он выводился за
 * небольшое время.
 */
void BarBase::paintVertexVector() {
    if (noPaint) {
        vertCnt = 0;
        return;
    }

    if (!vertCnt) {
        return;
    }

    // Настройка OpenGl вывода в состояние отрисовки буфера
    glEnableClientState(GL_VERTEX_ARRAY);
    // Установить двуменрный (2) буфер pVerts с элементами типа GL_FLOAT,
    // в котором точки лежат без смещений, одна за другой (0)
    glVertexPointer(2, GL_FLOAT, 0, pVerts);

    // RGB цвет фрактала - синий.
    glColor3ub(globR, globG, globB);

    // Вывести vertCnt элементов установленного буфера на экран
    // в виде множества линий (GL_LINES).
    // Начать вывод с 0-го элемента буфера (0).

    if (isPoints) {
        glDrawArrays(GL_POINTS, 0, vertCnt);
    }
    else {
        glDrawArrays(GL_LINES, 0, vertCnt);
    }
    // Сбросить счетчик буфера.
    vertCnt = 0;
}


/**
 * @brief Bar::draw
 * Виртуальный интерфейс для рисования фрактала.
 * Рисование начинается с точки (0.5, 0.5)
 */
void BarBase::draw(Qwt3D::Triple const&)
{
    vertCnt = 0;
    for (size_t i = 0; i < sizeof(pVerts) / sizeof(pVerts[0]); i++) {
        pVerts[i] = 0;
    }

    make_fractal();
}
