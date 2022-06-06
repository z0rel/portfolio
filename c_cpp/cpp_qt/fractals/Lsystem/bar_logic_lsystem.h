#ifndef BARLOGIC_LSYSTEM__H
#define BARLOGIC_LSYSTEM__H


#include <vector>

#include <QObject>
#include <QMainWindow>

#include "bar_base.h"


using namespace Qwt3D;


/**
 * @brief Класс Bar для рисований
 * Класс, реализующий функциональность рисования фрактала.
 * Предоставляет интерфейс рисования на другом оконном объекте.
 */
class BarLogicLSystem : public BarBase
{
public:
    /// Тип указателя на функции-члены для рисования фракталов.
    typedef void (BarLogicLSystem::*ruleCallback_t)();

    /// Параметры масштаба
    double Xmin = -50;
    double Xmax = 50;
    double Ymin = -50;
    double Ymax = 50;

    /// Указатель на текущу функцию рисования.
    ruleCallback_t axiomCallback;

    BarLogicLSystem(MainWindowBase *_parent)
        : BarBase(_parent),
          axiomCallback(&BarLogicLSystem::axiomFlower) { depth = 4; }

    void changeT(const ruleCallback_t _axiomCallback, size_t depth, int R, int G, int B);

    Qwt3D::Enrichment* clone() const { return new BarLogicLSystem(*this); }

    void axiomFlower();
    void axiomHylbert();
    void axiomDragon();
    void axiomChain();
    void axiomMatSerpinsky();
    void axiomHosper();
    void axiomPeano();
    void axiomSerpinsky();
    void axiomBush();
    void axiomMosaic();
    void axiomIsland();
    void axiomWeed();
    void axiomSnowflake();
    void axiomSnowflakeKosh();

protected:
    /// Тип состояния отрисовки.
    struct LSstate
    {
        LSstate() : x(0), y(0), alpha(0) {}

        LSstate(double _x, double _y, double _alpha)
            : x(_x), y(_y), alpha(_alpha) {}

        double x;
        double y;
        double alpha;
    };

    /// Текущий уровень вложенности рекурсии
    int currLvl;

    /// Текущая величина постоянного приращения угла
    double teta;

    /// Стек состояний
    std::vector<LSstate> lStack;

    /// Текущее и предыдущее состояния
    LSstate currState, prevState;

    /// Указатели на правила <F> <X> <Y> <b>
    ruleCallback_t fruleCallback, xruleCallback, yruleCallback, bruleCallback;

    bool verifyRecursionF();
    bool verifyRecursionB();

    void make_fractal();
    void outToScreen();

    void initGrammar(ruleCallback_t F,
                     ruleCallback_t X,
                     ruleCallback_t Y,
                     ruleCallback_t B,
                     double _teta,
                     double alpha = 0.0);

    void newFflower();

    void newFHylbert();
    void newXHylbert();
    void newYHylbert();

    void newFDragon();
    void newXDragon();
    void newYDragon();

    void newFChain();
    void newBChain();

    void newFMatSerpinsky();
    void newXMatSerpinsky();

    void newFHosper();
    void newXHosper();
    void newYHosper();

    void newFPeano();

    void newFSerpinsky();
    void newXSerpinsky();

    void newFBush();

    void newFMosaic();
    void newBMosaic();

    void newFIsland();

    void newFWeed();

    void newFSnowflake();

    void newFSnowflakeKosh();
};


#endif // BARLOGIC_LSYSTEM__H
