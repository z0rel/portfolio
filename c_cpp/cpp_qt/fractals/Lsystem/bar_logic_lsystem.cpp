#include <iostream>
#include <iomanip>
#include <math.h>

#include <qwt3d_color.h>
#include <qwt3d_plot3d.h>

#include "mainwindow_base.h"

#include "bar_logic_lsystem.h"
#include "qbitmap.h"


using namespace std;


/// Терминальный символ "+"
#define P currState.alpha += teta;

/// Терминальный символ "-"
#define M currState.alpha -= teta;

/// Терминальный символ "["
#define Open  lStack.push_back( currState );

/// Терминальный символ "]"
#define Close currState = lStack.back(); lStack.pop_back();

/// Общий макрос для грамматик <F> <X> <Y>
#define produceRuleF(name) \
    currLvl++; \
    if (verifyRecursionF()) { \
        (this->*name ## ruleCallback)(); \
    } \
    currLvl--

/// Общий макрос для грамматики <B>
#define produceRuleB(name) \
    currLvl++; \
    if (verifyRecursionB()) { \
        (this->*name ## ruleCallback)(); \
    } \
    currLvl--

/// Порождающая грамматика <F>
#define F produceRuleF(f);

/// Порождающая грамматика <X>
#define X produceRuleF(x);

/// Порождающая грамматика <Y>
#define Y produceRuleF(y);

/// Порождающая грамматика <b>
#define B produceRuleB(b);


/**
 * @brief BarLogicLSystem::verifyRecursionF
 *  Функция выполнения шага с отрисовкой для терминальных лексем <F> <X> <Y>.
 *  Проверяет находится ли лексема на последнем уровне рекурсии. Есл да - то она терминальная (непорождающий символ).
 * @return
 *  Нужно ли продолжать применять правила граммитики, или нет - символ нарисован и достигнут последний уровень рекурсии
 */
bool BarLogicLSystem::verifyRecursionF()
{
    if (currLvl > depth) {
        prevState = currState;
        currState.x += cos(currState.alpha);
        currState.y += sin(currState.alpha);
        outToScreen();
        Xmin = min(min(Xmin, prevState.x - 3), min(Xmin, currState.x - 3));
        Xmax = max(max(Xmax, prevState.x + 3), max(Xmax, currState.x + 3));
        Ymin = min(min(Ymin, prevState.y - 3), min(Ymin, currState.y - 3));
        Ymax = max(max(Ymax, prevState.y + 3), max(Ymax, currState.y + 3));
        return false;
    }
    return true;
}


/**
 * @brief BarLogic::verifyRecursionF
 *  Функция выполнения шага с терминальной лексемы <b>
 *  Проверяет находится ли лексема на последнем уровне рекурсии. Есл да - то она терминальная (непорождающий символ).
 * @return
 *  Нужно ли продолжать применять правила граммитики, или нет - достигнут последний уровень рекурсии
 */
bool BarLogicLSystem::verifyRecursionB()
{
    if (currLvl > depth) {
        currState.x += cos(currState.alpha);
        currState.y += sin(currState.alpha);
        return false;
    }
    return true;
}


/**
 * @brief BarLogicLSystem::initGrammar
 * Инициализация интерпретатора порождающей грамматики.
 * @param F_    Порождающее правило грамматики <F>
 * @param X_    Порождающее правило грамматики <X>
 * @param Y_    Порождающее правило грамматики <Y>
 * @param B_    Порождающее правило грамматики <b>
 * @param _teta Новый постоянный угол приращения
 * @param alpha Новый начальный угол
 */
void BarLogicLSystem::initGrammar(
        ruleCallback_t F_,
        ruleCallback_t X_,
        ruleCallback_t Y_,
        ruleCallback_t B_,
        double _teta,
        double alpha)
{
    fruleCallback = F_;
    xruleCallback = X_;
    yruleCallback = Y_;
    bruleCallback = B_;
    teta = _teta;
    currState = LSstate(0.0, 0.0, alpha);
}


/// Дракон Хайтера-Хайтвея
void BarLogicLSystem::newFDragon() { F }
void BarLogicLSystem::newXDragon() { X P Y F P }
void BarLogicLSystem::newYDragon() { M F X M Y }
void BarLogicLSystem::axiomDragon()
{
    using T = BarLogicLSystem;
    initGrammar(&T::newFDragon, &T::newXDragon, &T::newYDragon, 0, M_PI / 2.0);
    F X
}

/// Ковер Серпинского
void BarLogicLSystem::newFMatSerpinsky() { F F }
void BarLogicLSystem::newXMatSerpinsky() { M M F X F P P F X F P P F X F M M }
void BarLogicLSystem::axiomMatSerpinsky()
{
    using T = BarLogicLSystem;
    initGrammar(&T::newFMatSerpinsky, &T::newXMatSerpinsky, 0, 0, M_PI / 3.0);
    F X F M M F F M M F F
}


/// Кривая Гильберта, заполняющая плоскость
void BarLogicLSystem::newFHylbert() { F }
void BarLogicLSystem::newXHylbert() { M Y F P X F X P F Y }
void BarLogicLSystem::newYHylbert() { P X F M Y F Y M F X P }
void BarLogicLSystem::axiomHylbert()
{
    using T = BarLogicLSystem;
    initGrammar(&T::newFHylbert, &T::newXHylbert, &T::newYHylbert, 0, M_PI / 2.0);
    X
}


/// Кривая Госпера, заполняющая плоскость
void BarLogicLSystem::newFHosper() { F }
void BarLogicLSystem::newXHosper() { X P Y F P P Y F M F X M M F X F X M Y F P  }
void BarLogicLSystem::newYHosper() { M F X P Y F Y F P P Y F P F X M M F X M Y  }
void BarLogicLSystem::axiomHosper()
{
    using T = BarLogicLSystem;
    initGrammar(&T::newFHosper, &T::newXHosper, &T::newYHosper, 0, M_PI / 3.0);
    X F
}


/// Кривая Пеано, заполняющая плоскость
void BarLogicLSystem::newFPeano() { F M F P F P F P F P F M F M F M F P F }
void BarLogicLSystem::axiomPeano()
{
    initGrammar(&BarLogicLSystem::newFPeano, 0, 0, 0, M_PI / 4.0, M_PI / 4.0);
    F
}


/// Кривая Серпинского, заполняющая плоскость
void BarLogicLSystem::newFSerpinsky() { F }
void BarLogicLSystem::newXSerpinsky() { X F M F P F M X F P F P X F M F P F M X}
void BarLogicLSystem::axiomSerpinsky()
{
    using T = BarLogicLSystem;
    initGrammar(&T::newFSerpinsky, &T::newXSerpinsky, 0, 0, M_PI / 2.0, M_PI / 4.0);
    F P X F P F P X F
}


/// Куст
void BarLogicLSystem::newFBush() {
    M F P F P
    Open P F M F M Close
    M
    Open M F P F P F Close
}
void BarLogicLSystem::axiomBush()
{
    initGrammar(&BarLogicLSystem::newFBush, 0, 0, 0, M_PI / 8.0, M_PI / 2.0);
    F
}


/// Мозаика
void BarLogicLSystem::newFMosaic() { F M B P F F M F M F F M F B M F F P B M F F P F P F F P F B P F F F }
void BarLogicLSystem::newBMosaic() { B B B B B B }
void BarLogicLSystem::axiomMosaic() {
    using T = BarLogicLSystem;
    initGrammar(&T::newFMosaic, 0, 0, &T::newBMosaic, M_PI / 2.0);
    F M F M F M F
}


/// Остров
void BarLogicLSystem::newFIsland() { F P F M F M F F F P F P F M F }
void BarLogicLSystem::axiomIsland() {
    initGrammar(&BarLogicLSystem::newFIsland, 0, 0, 0, M_PI / 2.0);
    F P F P F P F
}


#define OFC Open F Close // OFC ::= [ F ]

/// Снежинка
void BarLogicLSystem::newFSnowflake() {
    F Open P P F Close Open M F F Close F F Open P F Close Open M F Close F F
}
void BarLogicLSystem::axiomSnowflake() {
    initGrammar(&BarLogicLSystem::newFSnowflake, 0, 0, 0, M_PI / 3.0);
    OFC P OFC P OFC P OFC P OFC P OFC
}


/// Снежинка Коха
void BarLogicLSystem::newFSnowflakeKosh() { F M F P P F M F }
void BarLogicLSystem::axiomSnowflakeKosh() {
    initGrammar(&BarLogicLSystem::newFSnowflakeKosh, 0, 0, 0, M_PI / 3.0);
    F P P F P P F
}


/// Сорняк
void BarLogicLSystem::newFWeed() { F Open P F Close F Open M F Close F }
void BarLogicLSystem::axiomWeed() {
    initGrammar(&BarLogicLSystem::newFWeed, 0, 0, 0, M_PI / 7.0, M_PI / 2.0);
    F
}

/// Цветок
void BarLogicLSystem::newFflower() {
    F F
    Open P P F Close
    Open P F   Close
    Open F     Close
    Open M F   Close
    Open M M F Close

}
void BarLogicLSystem::axiomFlower() {
    initGrammar(&BarLogicLSystem::newFflower, 0, 0, 0, M_PI / 16.0, M_PI / 2.0);
    F
    Open P F P F Close
    Open M F M F Close
    Open P P F   Close
    Open M M F   Close
    F
}


/// Цепочка
void BarLogicLSystem::newFChain() { F P B M F M F F F P F P B M F }
void BarLogicLSystem::newBChain() { B B B }
void BarLogicLSystem::axiomChain()
{
    using T = BarLogicLSystem;
    initGrammar(&T::newFChain, 0, 0, &T::newBChain, M_PI / 2.0);
    F P F P F P F
}


/**
 * @brief Bar::changeT
 * Интерфейс для изменения состояния следующей отрисовки
 * @param _axiomCallback  Указатель на новую функцию-член (аксиому) для отрисовки фрактала
 * @param _depth          Новая  глубина отрисовки фрактала
 * @param _R              Красная компонента цвета фрактала
 * @param _G              Зеленая компонента цвета фрактала
 * @param _B              Синяя   компонента цвета фрактала
 */
void BarLogicLSystem::changeT(const ruleCallback_t _axiomCallback, size_t _depth, int R_, int G_, int B_) {
    depth = _depth;
    axiomCallback = _axiomCallback;
    globR = R_;
    globG = G_;
    globB = B_;
}


/**
 * @brief outToScreen
 *  Отрисовка фигуры средствами OpenGl.
 *  Данные о линиях кладутся в буфер, который по достижении
 *  объема большего чем 1024 линии - выводится на экран. \n
 *  \n
 */
void BarLogicLSystem::outToScreen() {
    pVerts[vertCnt++] = prevState.x;
    pVerts[vertCnt++] = prevState.y;
    pVerts[vertCnt++] = currState.x;
    pVerts[vertCnt++] = currState.y;
    if (vertCnt > 1024 * 4) {
        paintVertexVector();
    }
}

/**
 * @brief make_fractal
 * Функция построения и отрисовки фрактала. \n
 */
void BarLogicLSystem::make_fractal()
{
    currLvl = 0;
    Xmin = 0.0;
    Xmax = 1.0;
    Ymin = 0.0;
    Ymax = 1.0;
    lStack.clear();

    (this->*axiomCallback)();
    // Освобождение буфера - выводит хвост, оставшийся в векторе вершин OpenGl
    paintVertexVector();

    parent->changeMinMax(Xmin, Xmax, Ymin, Ymax);
}
