#ifndef BAR_H
#define BAR_H

#include <stdint.h>

#include <QMainWindow>

#include "bar_base.h"


using namespace Qwt3D;


/**
 * @brief Класс Bar для рисований
 * Класс, реализующий функциональность рисования фрактала.
 * Предоставляет интерфейс рисования на другом оконном объекте.
 */
class BarLogic : public BarBase
{
public:
    /// Тип функций афинных преобразований
    typedef void (*Tfunc)(double x_res[2], double x[2]);

    /// Афинные преобразования для дерева
    static const Tfunc T_tree[5];
    /// Афинные преобразования для кристалла
    static const Tfunc T_crystal[4];
    /// Афинные преобразования для папоротника
    static const Tfunc T_fern[4];
    /// Афинные преобразования для ковра A
    static const Tfunc T_matA[3];
    /// Афинные преобразования для ковра B
    static const Tfunc T_matB[3];
    /// Афинные преобразования для ковра Серпинского
    static const Tfunc T_matS[3];
    /// Афинные преобразования для листа
    static const Tfunc T_leaf[2];

    BarLogic(MainWindowBase *parent)
        : BarBase(parent),
          afins(T_crystal),
          afinsSize(3) {}

    void changeT(const Tfunc *afins, size_t afinsSize, size_t depth, int R, int G, int B);

    Qwt3D::Enrichment* clone() const { return new BarLogic(*this); }

private:
    /// Текущий массив функций афинных преобразований
    const Tfunc * afins;

    /// Размер текущего массива функций афинных преобразваний - 1. Он равен количеству точек фигуры - 1.
    int afinsSize;

    /**
     * @brief data
     *  Массив данных с расчитанными отображениями на текущей ветви рекурсии.
     *    - @c data[i]      - Это точки, рассчитанные отображением T<i>
     *    - @c data[i][lvl] - Это точка, рассчитанная отображением @c T<i> на уровне @c lvl.
     *                          То есть data[i][lvl] = T<i>(data[i][lvl - 1]).
     *    - @c data[i][lvl][0] - это координата X
     *    - @c data[i][lvl][1] - это координата Y
     *    .
     *  При этом первые два уровня задаются специальным образом:
     *  @n
     *  Нулевой уровень: \n
     *  @code data[0][0] = data[1][0] = ... = data[N][0] = начальной точке (0.5, 0.5) @endcode \n
     *  Здесь N - это количество отображений @c T<i> \n
     *  \n
     *  Первый уровень:
     *  - @code data[0][0] = T<0>(0.5, 0.5)
     *  - @code data[1][0] = T<1>(0.5, 0.5)
     *  - ...
     *  - @code data[N][0] = T<N>(0.5, 0.5)
     *  .
     *  Данные первого уровня являтся исходной фигурой, на основе которой порождаются все остальные фигуры.
     */
    double data[7][50][2];


    void make_fractal();
    void makeT(int lvl, int currentState);
    void outToScreen(int lvl);
    void recursivePainting(int lvl);
};

#endif // BAR_H
