#include <iostream>
#include <iomanip>
#include <qbitmap.h>
#include <qwt3d_color.h>
#include <qwt3d_plot3d.h>

#include "bar_logic.h"


using namespace std;


/**
 * @brief Bar::changeT
 * Интерфейс для изменения состояния следующей отрисовки
 * @param _afins     Массив новых функций афинных преобразований
 * @param _afinsSize Размер массива afins
 * @param _depth     Новая глубина отрисовки фрактала
 * @param _R         Красная компонента цвета фрактала
 * @param _G         Зеленая компонента цвета фрактала
 * @param _B         Синяя   компонента цвета фрактала
 */
void BarLogic::changeT(const Tfunc * _afins, size_t _afinsSize, size_t _depth, int R, int G, int B ) {
    afins     = _afins;
    afinsSize = _afinsSize - 1;
    depth     = _depth;
    globR = R;
    globG = G;
    globB = B;
}


/**
 * @defgroup afina Афинные преобразования
 */
/** @{ */

/**
 * Компонент агрессивной оптимизации.
 * Макрос, определяющий двумерное афинное преобразование.
 */
#define T_templ(x_res, x, a, b, c, d, e, f) \
    x_res[0] = (a) * x[0] + (b) * x[1] + (e); \
    x_res[1] = (c) * x[0] + (d) * x[1] + (f)
// кристалл
void T1_crystal(double x_res[2], double x[2]) { T_templ(x_res, x, 0.255,  0.0  , 0.0  , 0.255, 0.3726,  0.6714); }
void T2_crystal(double x_res[2], double x[2]) { T_templ(x_res, x, 0.255,  0.0  , 0.0  , 0.255, 0.1146,  0.2232); }
void T3_crystal(double x_res[2], double x[2]) { T_templ(x_res, x, 0.255,  0.0  , 0.0  , 0.255, 0.6306,  0.2232); }
void T4_crystal(double x_res[2], double x[2]) { T_templ(x_res, x, 0.37 , -0.642, 0.642, 0.370, 0.6356, -0.0061); }

// папоротник
void T1_fern(double x_res[2], double x[2]) { T_templ(x_res, x, 0.7,  0.000,  0.0000, 0.70, 0.1496, 0.2962); }
void T2_fern(double x_res[2], double x[2]) { T_templ(x_res, x, 0.1, -0.433,  0.1732, 0.25, 0.4478, 0.0014); }
void T3_fern(double x_res[2], double x[2]) { T_templ(x_res, x, 0.1,  0.433, -0.1732, 0.25, 0.4445, 0.1559); }
void T4_fern(double x_res[2], double x[2]) { T_templ(x_res, x, 0.0,  0.000,  0.0000, 0.30, 0.4987, 0.0070); }

// ковер А
void T1_matA(double x_res[2], double x[2]) { T_templ(x_res, x,  0.5,  0.0,  0.0, -0.5, 0.5, 0.5); }
void T2_matA(double x_res[2], double x[2]) { T_templ(x_res, x,  0.0, -0.5, -0.5,  0.0, 0.5, 0.5); }
void T3_matA(double x_res[2], double x[2]) { T_templ(x_res, x, -0.5,  0.0,  0.0, -0.5, 0.5, 1.0); }

// ковер B
void T1_matB(double x_res[2], double x[2]) { T_templ(x_res, x,  0.5,  0.0,  0.0, -0.5, 0.0, 1.0); }
void T2_matB(double x_res[2], double x[2]) { T_templ(x_res, x,  0.0,  0.5,  0.5,  0.0, 0.0, 0.0); }
void T3_matB(double x_res[2], double x[2]) { T_templ(x_res, x,  0.5,  0.0,  0.0,  0.5, 0.5, 0.0); }

// лист
void T1_leaf(double x_res[2], double x[2]) { T_templ(x_res, x,  0.4,  -0.3733,  0.0600, 0.6, 0.3533, 0.0); }
void T2_leaf(double x_res[2], double x[2]) { T_templ(x_res, x, -0.8,  -0.1867,  0.1371, 0.8, 1.1000, 0.1); }

// ковер Серпинского
void T1_matS(double x_res[2], double x[2]) { T_templ(x_res, x,  0.5,  0.0,  0.0, 0.5, 0.00, 0.000); }
void T2_matS(double x_res[2], double x[2]) { T_templ(x_res, x,  0.5,  0.0,  0.0, 0.5, 0.50, 0.000); }
void T3_matS(double x_res[2], double x[2]) { T_templ(x_res, x,  0.5,  0.0,  0.0, 0.5, 0.25, 0.433); }

// дерево
void T1_tree(double x_res[2], double x[2]) { T_templ(x_res, x,  0.1950, -0.4880,  0.3440,  0.4430, 0.4431, 0.2452); }
void T2_tree(double x_res[2], double x[2]) { T_templ(x_res, x,  0.4620,  0.4140, -0.2520,  0.3610, 0.2511, 0.5692); }
void T3_tree(double x_res[2], double x[2]) { T_templ(x_res, x, -0.0580, -0.0700,  0.4530, -0.1110, 0.5976, 0.0969); }
void T4_tree(double x_res[2], double x[2]) { T_templ(x_res, x, -0.0350,  0.0700, -0.4690,  0.0220, 0.4884, 0.5069); }
void T5_tree(double x_res[2], double x[2]) { T_templ(x_res, x, -0.6370,  0.0000,  0.0000,  0.5010, 0.8562, 0.2513); }

const BarLogic::Tfunc BarLogic::T_tree   [5] = { T1_tree   , T2_tree   , T3_tree   , T4_tree   , T5_tree};
const BarLogic::Tfunc BarLogic::T_crystal[4] = { T1_crystal, T2_crystal, T3_crystal, T4_crystal };
const BarLogic::Tfunc BarLogic::T_fern   [4] = { T1_fern   , T2_fern   , T3_fern   , T4_fern    };
const BarLogic::Tfunc BarLogic::T_matA   [3] = { T1_matA   , T2_matA   , T3_matA};
const BarLogic::Tfunc BarLogic::T_matB   [3] = { T1_matB   , T2_matB   , T3_matB};
const BarLogic::Tfunc BarLogic::T_matS   [3] = { T1_matS   , T2_matS   , T3_matS};
const BarLogic::Tfunc BarLogic::T_leaf   [2] = { T1_leaf   , T2_leaf};

/** @} */

/**
 * @brief outToScreen
 *  Отрисовка фигуры средствами OpenGl.
 *  Данные о линиях кладутся в буфер, который по достижении
 *  объема большего чем 1024 линии - выводится на экран. \n
 *  \n
 *  Для ковров (figureSize == 2) рисутся замкнутые треугольники.
 *  Рисутся линиями @c GL_LINES, а не треугольниками @c GL_TRIANGLE
 *  т.к. во втором случае выводится менее четкая картинка. \n
 *  Для всех остальных аттракторов рисуется звездообразная
 *  фигура - центарльная точка + отрезки, исходящи из нее.
 *
 * @param lvl
 *  Уровень текущего узла, который предоставляет точки фигуры.
 */
void BarLogic::outToScreen(int lvl)
{
    if (isPoints) { // Оптимизация для поточечного вывода
        for (int i = 1; i <= afinsSize; i++) {
            pVerts[vertCnt++] = data[i][lvl][0];
            pVerts[vertCnt++] = data[i][lvl][1];
        }
    }
    else {
        if (afinsSize == 2 ) { // ковры
            pVerts[vertCnt++] = data[0][lvl][0];
            pVerts[vertCnt++] = data[0][lvl][1];
            for (int i = 1; i <= afinsSize; i++) {
                pVerts[vertCnt++] = data[i][lvl][0];
                pVerts[vertCnt++] = data[i][lvl][1];
                pVerts[vertCnt++] = data[i][lvl][0];
                pVerts[vertCnt++] = data[i][lvl][1];
            }
            pVerts[vertCnt++] = data[0][lvl][0];
            pVerts[vertCnt++] = data[0][lvl][1];
        }
        else { // звёздочка, дерево, куст и лист
            for (int i = 0; i < afinsSize; i++) {
                pVerts[vertCnt++] = data[afinsSize][lvl][0];
                pVerts[vertCnt++] = data[afinsSize][lvl][1];
                pVerts[vertCnt++] = data[i][lvl][0];
                pVerts[vertCnt++] = data[i][lvl][1];
            }
        }
    }

    if (vertCnt > 1024 * 4)
        paintVertexVector();
}

/**
 * @brief makeT
 * Вычисление нового значения данных на узле, соответственно
 * выбранному афинному преобразованию.
 * @param lvl
 *   Текущий уровень, на котором применяется афинное преобразование.
 * @param currentState
 *   Состяние уровня. Соответствует индексу применямого афинного преобразования.
 */
void BarLogic::makeT(int lvl, int currentState)
{
    for (int i = 0; i <= afinsSize; i++) {
        afins[currentState](data[i][lvl], data[i][lvl - 1]);
    }
}

/**
 * @brief Bar::recursivePainting
 * Функция рекурсивного построения фрактала.
 * @param lvl
 */
void BarLogic::recursivePainting(int lvl) {
    lvl++;
    if (lvl > depth) {
        return;
    }

    for (int i = 0; i <= afinsSize; i++) {
        makeT(lvl, i);
        if (afins == T_crystal
             || lvl == depth
             || (afins == T_fern && !isPoints && lvl >= depth - 3)) {
            outToScreen(lvl);
        }
        recursivePainting(lvl);
    }
}

/**
 * @brief make_fractal
 * Функция построения и отрисовки фрактала. \n
 * С помощью afinsSize афинных преобразований генерирует afinsSize начальных точек.
 * Эти точки образуют исходную фигуру:
 *  - отрезок для листа;
 *  - треугольник для ковров;
 *  - букву Y для кристалла;
 *  - Стрелку ↓ для папоротника;
 *  - Букву X для дерева;
 *  .
 *  Эта фигура затем, с помощью афинных преобразований будет преобразована
 *  во всевозможные другие фигуры такой же формы. Эти фигуры и будут выведены на экран.
 *  Для кристалла и папоротника должны выводиться все фигуры, а для всех остальных аттракторов -
 *  только фигуры последнего уровня глубины.
 *
 * @param x
 *    Начальная точка, на основе которой генерируется исходная фигура для построения фрактала
 */
void BarLogic::make_fractal()
{
    double x[2] = {0.5, 0.5};
    for (int i = 0; i <= afinsSize; i++) {
        data[i][0][0] = x[0];
        data[i][0][1] = x[1];
        // Построение начальной фигуры: X1_0 = T0(X0); X1_1 = T1(X0); X1_2 = T2(X0) ...
        afins[i](data[i][1], x);
    }

    // Для кристалла нужно рисовать все уровни
    if (afins == T_crystal) {
        outToScreen(1);
    }
    recursivePainting(1);

    // Освобождение буфера - выводит хвост, оставшийся в векторе вершин OpenGl
    paintVertexVector();
}
