#include "temperature.h"

#include <global.h>


Temperature::Temperature(TintMatr A1, TintMatr A2, TintMatr A3, int dim3, double ThirdKindCoeff[7][5],
bool to_file)
: output_to_file(to_file)
{
    K = NULL;
    F = NULL;
    this->dim3 = dim3;
    for (int i = 0; i < 7; i++)
    {
        memcpy(this->ThirdKindCoeff[i], ThirdKindCoeff[i], sizeof(double) * 5);
    }
    setA123(A1, A2, A3);
}


void Temperature::freeObj()
{

}


Temperature::~Temperature()
{
    freeObj();
}


long double Temperature::b (int index)
{
    return -Tetr[index + 2][3] * Tetr[index + 3][4] + Tetr[index + 3][3] * Tetr[index + 2][4]
           +Tetr[index + 1][3] * Tetr[index + 3][4] - Tetr[index + 3][3] * Tetr[index + 1][4]
           -Tetr[index + 1][3] * Tetr[index + 2][4] + Tetr[index + 2][3] * Tetr[index + 1][4];
}


long double Temperature::c (int index)
{
    return +Tetr[index + 2][2] * Tetr[index + 3][4] - Tetr[index + 3][2] * Tetr[index + 2][4]
           -Tetr[index + 1][2] * Tetr[index + 3][4] + Tetr[index + 3][2] * Tetr[index + 1][4]
           +Tetr[index + 1][2] * Tetr[index + 2][4] - Tetr[index + 2][2] * Tetr[index + 1][4];
}

long double Temperature::d (int index)
{
    return -Tetr[index + 2][2] * Tetr[index + 3][3] + Tetr[index + 3][2] * Tetr[index + 2][3]
           +Tetr[index + 1][2] * Tetr[index + 3][3] - Tetr[index + 3][2] * Tetr[index + 1][3]
           -Tetr[index + 1][2] * Tetr[index + 2][3] + Tetr[index + 2][2] * Tetr[index + 1][3];
}

long double Temperature::det (int index)
{
    return ( Tetr[index + 1][2] - Tetr[index][2]) *
            ((Tetr[index + 2][3] - Tetr[index][3]) * (Tetr[index + 3][4] - Tetr[index][4]) -
            (Tetr[index + 3][3] - Tetr[index][3]) * (Tetr[index + 2][4] - Tetr[index][4] ))-
            ( Tetr[index + 2][2] - Tetr[index][2]) *
            ((Tetr[index + 1][3] - Tetr[index][3]) * (Tetr[index + 3][4] - Tetr[index][4]) -
            (Tetr[index + 3][3] - Tetr[index][3]) * (Tetr[index + 1][4] - Tetr[index][4] ))+
            ( Tetr[index + 3][2] - Tetr[index][2]) *
            ((Tetr[index + 1][3] - Tetr[index][3]) * (Tetr[index + 2][4] - Tetr[index][4])-
            (Tetr[index + 2][3] - Tetr[index][3]) * (Tetr[index + 1][4] - Tetr[index][4] ));
}

void Temperature::setA123(TintMatr A1, TintMatr A2, TintMatr A3)
{
    this->A1 = A1;
    this->A2 = A2;
    this->A3 = A3;
}

bool Temperature::firstKind(int s)
{
    for (int k = 1; k <= dim3; k++)
        if (s == A3[k][1])
            return true;
    return false;
}

void Temperature::correctMatrix(double hh, double qq, double TTo, int ii, int jj, int kk)
{
    K[ii][ii] += hh / 12.0;
    K[jj][jj] += hh / 12.0;
    K[kk][kk] += hh / 12.0;
    K[ii][jj] += hh / 24.0;
    K[jj][ii] += hh / 24.0;
    K[ii][kk] += hh / 24.0;
    K[jj][kk] += hh / 24.0;
    K[kk][ii] += hh / 24.0;
    K[kk][jj] += hh / 24.0;
    F[ii]     += hh * TTo / 6.0 + qq / 6.0;
    F[jj]     += hh * TTo / 6.0 + qq / 6.0;
    F[kk]     += hh * TTo / 6.0 + qq / 6.0;
}

void Temperature::FindBound(int &ii, int &jj, int &kk)
{
    int x1 = A2[ii][2];
    int x2 = A2[jj][2];
    int x3 = A2[kk][2];

    if (x1 == x2 && x2 == x3)
    {
        if (!x1 || x1 == M1) /// выходит на границу грань
        {
            if (!firstKind(ii) || !firstKind(jj) || !firstKind(kk))
            {   /// значит это условие 3 рода, надо добавлять интегралы
                if (!x1) /// на 5 грани условие надо брать
                {
                    h  = ThirdKindCoeff[5][2];
                    q  = ThirdKindCoeff[5][4];
                    T0 = ThirdKindCoeff[5][3];
                }
                else /// на 3 грани
                {
                    h  = ThirdKindCoeff[3][2];
                    q  = ThirdKindCoeff[3][4];
                    T0 = ThirdKindCoeff[3][3];
                }
                correctMatrix(h, q, T0, ii, jj, kk);
            }
        }
    }
    else   /// проверяем равенство y
    {
        int y1 = A2[ii][3];
        int y2 = A2[jj][3];
        int y3 = A2[kk][3];

        if (y1 == y2 && y2 == y3)
        {
            if (!y1 || y1 == N1) /// выходит на границу грань
            {
                if (!firstKind(ii) || !firstKind(jj) || !firstKind(kk))
                {
                    /// значит это условие 3 рода, надо добавлять интегралы
                    if (!y1) /// на 4 грани условие надо брать
                    {
                        h  = ThirdKindCoeff[4][2];
                        q  = ThirdKindCoeff[4][4];
                        T0 = ThirdKindCoeff[4][3];
                    }
                    else /// на 2 грани
                    {
                        h  = ThirdKindCoeff[2][2];
                        q  = ThirdKindCoeff[2][4];
                        T0 = ThirdKindCoeff[2][3];
                    }
                    correctMatrix(h, q, T0, ii, jj, kk);
                }
            }
        }
        else
        {
            int z1 = A2[ii][4];
            int z2 = A2[jj][4];
            int z3 = A2[kk][4];

            if (z1 == z2 && z2 == z3)
            {
                if (!z1 || z1 == K1) /// выходит на границу грань
                {
                    if (!firstKind(ii) || !firstKind(jj) || !firstKind(kk))
                    {   /// значит это условие 3 рода, надо добавлять интегралы
                        if (!z1) /// на 6 грани условие надо брать
                        {
                            h  = ThirdKindCoeff[6][2];
                            q  = ThirdKindCoeff[6][4];
                            T0 = ThirdKindCoeff[6][3];
                        }
                        else  /// на 1 грани
                        {
                            h  = ThirdKindCoeff[1][2];
                            q  = ThirdKindCoeff[1][4];
                            T0 = ThirdKindCoeff[1][3];
                        }
                        correctMatrix(h, q, T0, ii, jj, kk);
                    }
                }
            }
        }
    }
}


/// Формирование тетраэдров
void Temperature::MakeTetraiders(int dim1, TintMatr A1, TintMatr &TetrMatr)
{
    /// dim2 - число узлов;  dim1 - число кирпичиков
    setlength<int>(TetrMatr, dim1 * 5, 4);
    int i_start, i_curr;

    for (int i = 1; i <= dim1; i++)
    {
        i_start = (i - 1) * 5;
        i_curr = i_start;
        TetrMatr[i_curr][0] = A1[i][1 + 1];
        TetrMatr[i_curr][1] = A1[i][0 + 1];
        TetrMatr[i_curr][2] = A1[i][6 + 1];
        TetrMatr[i_curr][3] = A1[i][2 + 1];
        i_curr++;
        TetrMatr[i_curr][0] = A1[i][3 + 1];
        TetrMatr[i_curr][1] = A1[i][4 + 1];
        TetrMatr[i_curr][2] = A1[i][0 + 1];
        TetrMatr[i_curr][3] = A1[i][2 + 1];
        i_curr++;
        TetrMatr[i_curr][0] = A1[i][5 + 1];
        TetrMatr[i_curr][1] = A1[i][2 + 1];
        TetrMatr[i_curr][2] = A1[i][6 + 1];
        TetrMatr[i_curr][3] = A1[i][4 + 1];
        i_curr++;
        TetrMatr[i_curr][0] = A1[i][7 + 1];
        TetrMatr[i_curr][1] = A1[i][0 + 1];
        TetrMatr[i_curr][2] = A1[i][4 + 1];
        TetrMatr[i_curr][3] = A1[i][6 + 1];
        i_curr++;
        TetrMatr[i_curr][0] = A1[i][0 + 1];
        TetrMatr[i_curr][1] = A1[i][2 + 1];
        TetrMatr[i_curr][2] = A1[i][4 + 1];
        TetrMatr[i_curr][3] = A1[i][6 + 1];
    }

    if (output_to_file)
    {
        outMatrToFile<int>("Tetraiders.txt", TetrMatr, 0, dim1 * 5 - 1, 0, 3);
    }
}


void Temperature::buildMatrK_andVectF(TintMatr A2, TintMatr A3, int dim3, TrealMatr &K, TVector &F,
                                      int dim1, int dim2, TintMatr TetrMatr)
{
    freeObj();

    dim2_Nuzel = dim2;
    dim1_Nkub = dim1;

    int Ntetr = dim1_Nkub * 5;

    setlength<long double>(K, dim2_Nuzel + 1, dim2_Nuzel + 2);
    setlength<long double>(F, dim2_Nuzel + 1);

    this->F = F;
    this->K = K;

    for (int i = 1; i <= Ntetr; i++)
    {
        /// Tetr[*][1] - номер узла
        /// Tetr[*][2] - координата x
        /// Tetr[*][3] - координата y
        /// Tetr[*][4] - координата z
        /// Tetr[*][5] - номер строки(столбца) в матрице
        for (int j = 1; j <= 4; j++)          /// заполняем номера вершин тетраэдров
        {
            Tetr[j][1] = TetrMatr[i-1][j-1];
        }

        for (int l = 1; l <= 4; l++)
        {
            for (int j = 1; j <= dim2_Nuzel; j++)
            {
                if (Tetr[l][1] == A2[j][1])
                {
                    /// заполняем координаты x,y,z у тетраэдров
                    Tetr[l][2] = A2[j][2];
                    Tetr[l][3] = A2[j][3];
                    Tetr[l][4] = A2[j][4];
                }
                if (Tetr[l][1] == A2[j][1])
                {
                    Tetr[l][5] = A2[j][1];
                }
            }
        }
        /// для циклических перестановок
        for (int j = 1; j <= 5; j++)
        {
            Tetr[5][j] = Tetr[1][j];
            Tetr[6][j] = Tetr[2][j];
            Tetr[7][j] = Tetr[3][j];
        }

        long double v;
        if ( i % 5 == 0 )
        {
            v = 1.0 / 3.0;
        }
        else
        {
            v = 1.0 / 6.0;
            /// проверяем, выходит  ли тетраэдр гранями на границу бруса
            /// и считаем интергады для условия 3 рода
            FindBound(Tetr[1][1], Tetr[2][1], Tetr[3][1]);
            FindBound(Tetr[3][1], Tetr[4][1], Tetr[5][1]);
            FindBound(Tetr[4][1], Tetr[5][1], Tetr[6][1]);
        }

        for (int l = 1; l <= 4; l++)
        {
            for (int j = 1; j <= 4; j++) /// считаем обобщенную матрицу, v - объем тетраэдра
            {
                index1 = Tetr[l][5];
                index2 = Tetr[j][5];

                long double sum = ( b(l) * b(j) + c(l) * c(j) + d(l) * d(j) ) / ( det(l) * det(j) );
                K[index1][index2] = K[index1][index2] + v * sum;
            }
        }
    }

    /// учёт граничных условий 1 рода
    int *delindex;

    setlength<int>(delindex, dim2_Nuzel + 1);

    /// delindex - строки матрицы, в которых уже учтены начальные условия
    for (int i = 1; i <= dim3; i++)
    {
        /// поиск соответствующей строки для граничного условия
        for (int j = 1; j <= dim2_Nuzel; j++)
        {
            if ( A3[i][1] == A2[j][1] )
            {
                index1 = A2[j][1];
                break;
            }
        }
        /// изменение правой части
        F[index1] = K[index1][index1] * A3[i][2];

        for (int j = 1; j <= dim2_Nuzel; j++)
        {
            if (j != index1 && !delindex[j])
            {
                F[j] = F[j] - K[j][index1] * A3[i][2];
            }
        }
        /// обнуление в матрице
        for (int j = 1; j <= dim2_Nuzel; j++)
        {
            if ( j != index1 )
            {
                K[j][index1] = 0;
            }
        }
        for (int j = 1; j <= dim2_Nuzel; j++)
        {
            if (j != index1)
            {
                K[index1][j] = 0;
            }
        }
        delindex[index1] = 1;
    }
    free_matr<int>(delindex);
}


void Temperature::TemperatureToFile(TVector g)
{
    QFile res_f("res.txt");
    if (!res_f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream res(&res_f);

    unsigned long long count_nodes = (N1 + 1) * (M1 + 1) * (K1 + 1);
    res << count_nodes << "\n";

    res.setRealNumberNotation(QTextStream::FixedNotation);
    res.setRealNumberPrecision(8);
    res.setFieldAlignment(QTextStream::AlignRight);
    res.setFieldWidth(18);

    count_nodes = (count_nodes > 100500) ? 100500 : count_nodes;
    for (unsigned long long i = 1; i <= count_nodes; i++)
    {
        res << i << " " << (double)(g[i]) << "\n";
    }
    res.flush();
    res_f.flush();
    res_f.close();
}


int Temperature::i_n(int i, int n, int lenta)
{
    return (( i <= (n - lenta) ) ? i + lenta - 1 : n); /// +1
}


int Temperature::i0(int i, int lenta)
{
    return (( i <= lenta ) ? 1 : i - lenta + 1);
}


int Temperature::L(TrealMatr K, int dim2)
{
    int max = 0;
    int j;

    for (int i = 1; i <= dim2; i++)
    {
        for (j = dim2; j >= i && !K[i][j]; j--)
        {
        }

        if (K[i][j] && ((j - i + 1) > max))
        {
            max = j - i + 1;
        }
    }
    return max;
}


/**
  * Решение матрицы методом Холецкого.
  * @param K    - построенная в функции buildMatrK_andVectF матрица K
  * @param dim2 - количество узлов
  * @param f    - вектор правой части, построенный в функции buildMatrK_andVectF
  * @param g    - вектор - решение
  */
void Temperature::HaletskiiMethod(TrealMatr K, int dim2, TVector F, TVector &g)
{
    double s;
    TrealMatr b;
    TrealMatr c_arr;
    TrealMatr AN;
    TVector y;
    int lenta;

    if (output_to_file)
    {
        outMatrToFile<long double>("K.txt", K, 1, dim2, 1, dim2 + 1);
    }

    lenta = L(K, dim2);

    /// в конце освободить F и X
    setlength<long double>(g, dim2 + 1);

    /// локальные динамические массивы
    setlength<long double>(y,     dim2 + 1);
    setlength<long double>(b,     dim2 + 1, lenta + 2);
    setlength<long double>(c_arr, dim2 + 1, lenta + 2);
    setlength<long double>(AN,    dim2 + 1, lenta + 2);

    for (int i = 1; i <= dim2; i++)
    {
        for (int j = 1; j <= lenta; j++)
        {
            AN[i][j] = 0;
        }
    }
    for (int i = 1; i <= lenta - 1; i++)
    {
        for (int j = lenta - i + 1; j <= lenta+1; j++)
        {
            AN[i][j] = K[i][j + i - lenta];
        }
    }

    for (int i = lenta; i <= dim2; i++)
    {
        for (int j = 1; j <= lenta + 1; j++)
        {
            AN[i][j] = K[i][j + i - lenta];
        }
    }

    /// инициализация матрицы с
    for (int i = 1; i <= dim2; i++)
    {
        c_arr[i][1] = 1;
    }

    /// прямой ход
    for (int j = 1; j <= dim2; j++)
    {
        for (int i = j; i <= i_n(j, dim2, lenta); i++)
        {
            s = AN[i][j - i + lenta];
            for (int k = i0(i, lenta); k <= j - 1; k++)
            {
                s = s - b[i][k - i + lenta] * c_arr[k][j - k + 1];
            }
            b[i][j - i + lenta] = s;
        }
        for (int i = j + 1; i <= i_n(j, dim2, lenta); i++)
        {
            s = AN[i][lenta + j - i];
            for (int k = i0(i, lenta); k <= j - 1; k++)
            {
                s = s - b[j][k - j + lenta] * c_arr[k][i - k + 1];
            }
            c_arr[j][i - j + 1] = s / b[j][lenta];
        }
    }

    /// обратный ход
    y[1] = F[1] / b[1][lenta];
    for (int i = 2; i <= dim2; i++)
    {
        s = F[i];
        for (int k = i0(i, lenta); k <= i - 1; k++)
        {
            s = s - b[i][k - i + lenta] * y[k];
        }
        y[i] = s / b[i][lenta];
    }

    g[dim2] = y[dim2];
    for (int i = dim2 - 1; i >= 1; i--)
    {
        s = y[i];
        for (int k = i + 1; k <= i_n( i, dim2, lenta ); k++)
        {
            s = s - c_arr[i][k - i + 1] * g[k];
        }
        g[i] = s;
    }

    if (output_to_file)
    {
        outMatrToFile<long double>("AN.txt", AN, 1, dim2, 1, lenta + 1);
        outMatrToFile<long double>("b_arr.txt", b, 1, dim2, 1, lenta + 1);
        outMatrToFile<long double>("c_arr.txt", c_arr, 1, dim2, 1, lenta + 1);
        outVectToFile<long double>("y.txt", y, 1, dim2);
        TemperatureToFile(g);
    }

    free_matr<long double>(y);
    free_matr<long double>(c_arr,  dim2 + 1);
    free_matr<long double>(b,  dim2 + 1);
    free_matr<long double>(AN, dim2 + 1);
}
