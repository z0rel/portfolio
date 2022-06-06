#include "maketetraidersobj.h"


MakeTetraidersObj::MakeTetraidersObj(double ThirdKindCoeff[7][5], bool to_file)
	: output_to_file(to_file)
{
    for (int i = 0; i < 7; i++)
    {
		memcpy(this->ThirdKindCoeff[i], ThirdKindCoeff[i], sizeof(double) * 5);
    }
}


void MakeTetraidersObj::MakeCube(int N, int M, int K, TintMatr &A1, TintMatr &A2, TintMatr &A3,
                                 int &dim1, int &dim2, int &dim3)
{
	CreateKub(N, M, K, A1, A2, A3, dim1, dim2, dim3);

	if (output_to_file)
	{
		outMatrToFile<int>("A1.txt", A1, 1, dim1, 1, 8);
		outMatrToFile<int>("A2.txt", A2, 1, dim2, 1, 4);
		outMatrToFile<int>("A3.txt", A3, 1, dim3, 1, 2);
	}
}


int MakeTetraidersObj::bounded(int x, int y, int z, int n)
{
	switch ((int)(ThirdKindCoeff[n][2]))
	{
    case const_func:
        return (int)round(ThirdKindCoeff[n][3]);
    case x_y_z_func:
        return x + y + z;
    case x_func:
        return x;
    case y_func:
        return y;
    case z_func:
        return z;
    case x_z_func:
        return x + z;
    case x3_y3_z3_func:
        return x * x * x + y * y * y + z * z * z;
    case x2_y2_z2_20_func:
        return x * x + y * y + z * z + 20;
	}
}


void MakeTetraidersObj::corner(int i, int j, int l, TintMatr A2, TintMatr A3, int &por,
							   int N, int M, int K)
{
  if ( (ThirdKindCoeff[i][1] == 0) || (ThirdKindCoeff[j][1] == 0) || (ThirdKindCoeff[l][1] == 0) )
	{
      if ((i == 1) && (j == 2) && (l == 5))
      {
          A3[por][1] = N + 1;
      }
      if ((i == 1) && (j == 4) && (l == 5))
      {
          A3[por][1] = 1;
      }
      if ((i == 1) && (j == 2) && (l == 3))
      {
          A3[por][1] = (M + 1) * (N + 1);
      }
      if ((i == 1) && (j == 3) && (l == 4))
      {
          A3[por][1] = (N + 1) * M + 1;
      }
      if ((i == 6) && (j == 2) && (l == 5))
      {
          A3[por][1] = K * (N + 1) * (M + 1) + N + 1;
      }
      if ((i == 6) && (j == 4) && (l == 5))
      {
          A3[por][1] = K * (N + 1) * (M + 1) + 1;
      }
      if ((i == 6) && (j == 2) && (l == 3))
      {
          A3[por][1] = K * (N + 1) * (M + 1) + (M + 1) * (N + 1);
      }
      if ((i == 6) && (j == 3) && (l == 4))
      {
          A3[por][1] = K * (N + 1) * (M + 1) + (N + 1) * M + 1;
      }

      int koord = !ThirdKindCoeff[i][1] ? i : (!ThirdKindCoeff[j][1] ? j : l);

	  A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], koord);
	  por++;
	}
}


/// ===========================================
void MakeTetraidersObj::xparallelside(int i, int l, TintMatr A2, TintMatr A3,
									  int &por, int N, int M, int K)
{
    if (!ThirdKindCoeff[i][1] || !ThirdKindCoeff[l][1])
    {
        for (int j = 2; j <= M; j++)
        {
            if (i == 1 && l == 2)
            {
                A3[por][1] = j * (N + 1);
            }
            if (i == 1 && l == 4)
            {
                A3[por][1] = (j - 1) * (N + 1) + 1;
            }
            if (i == 6 && l == 2)
            {
                A3[por][1] = K * (M + 1) * (N + 1) + j * (N + 1);
            }
            if (i == 6 && l == 4)
            {
                A3[por][1] = K * (M + 1) * (N + 1) + (j - 1) * (N + 1) + 1;
            }
            int koord = ((ThirdKindCoeff[i][1] == 0 ) ?  i :  l);
            A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], koord);
            por++;
		}
	}
}


/// ===========================================
void MakeTetraidersObj::yparallelside(int j, int l, TintMatr A2, TintMatr A3,
									  int &por, int N, int M, int K)
{
    if ( (ThirdKindCoeff[j][1] == 0) || (ThirdKindCoeff[l][1] == 0))
    {
        for (int i = 2; i <= N; i++)
        {
            if ((j == 1) && (l == 3))
            {
                A3[por][1] = M * (N + 1) + i;
            }
            if ((j == 1) && (l == 5))
            {
                A3[por][1] = i;
            }
            if ((j == 6) && (l == 3))
            {
                A3[por][1] = K * (M + 1) * (N + 1) + M * (N + 1) + i;
            }
            if ((j == 6) && (l == 5))
            {
                A3[por][1] = K * (M + 1) * (N + 1) + i;
            }
            int koord = !ThirdKindCoeff[j][1] ? j : l;

            A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], koord);
            por++;
        }
    }
}


/// N, M, K - размерности разбиений куба по осям
/// K - также определяет количество слоев
void MakeTetraidersObj::CreateKub(int N, int M, int K,
                                  TintMatr &A1, TintMatr &A2, TintMatr &A3,
                                  int &dim1, int &dim2, int &dim3)
{
	int por;

	/// Матрица A1 - является матрицей смежности. Ее формат таков:
	/// - A1[][1] - № узла, передний левый,   нижний,  смежный с A1[][8]
	/// - A1[][2] -         передний правый,  нижний,  смежный с A1[][7]
	/// - A1[][3] -         передний правый,  верхний, смежный с A1[][6]
	/// - A1[][4] -         передний левый,   верхний, смежный с A1[][5]
	/// - A1[][5] - № узла, задний   верхний, левый,   смежный с A1[][4]
	/// - A1[][6] -         задний   верхний, правый,  смежный с A1[][3]
	/// - A1[][7] -         задний   нижний,  правый,  смежный с A1[][2]
	/// - A1[][8] -         задний   нижний,  левый,   смежный с A1[][1]
	///
	setlength<int>(A1, N * M * K + 1, 9);

	por = 1;
    for (int i = 0; i < K; i++)
    {
        for (int j = 0; j < M; j++)
        {
			for (int l = 1; l <= N; l++)
			{
				A1[por][1] = i * (N + 1) * (M + 1) + j * (N + 1) + l;
				A1[por][2] = i * (N + 1) * (M + 1) + j * (N + 1) + l + N + 1;
				A1[por][3] = i * (N + 1) * (M + 1) + j * (N + 1) + l + N + 2;
				A1[por][4] = i * (N + 1) * (M + 1) + j * (N + 1) + l + 1;
				A1[por][5] = i * (N + 1) * (M + 1) + j * (N + 1) + l + (M + 1) * (N + 1) + 1;
				A1[por][6] = i * (N + 1) * (M + 1) + j * (N + 1) + l + (M + 1) * (N + 1) + N + 2;
				A1[por][7] = i * (N + 1) * (M + 1) + j * (N + 1) + l + (M + 1) * (N + 1) + N + 1;
				A1[por][8] = i * (N + 1) * (M + 1) + j * (N + 1) + l + (M + 1) * (N + 1);
				por++;
			}
        }
    }

	dim1 = por - 1;

	por = 1;
	setlength<int>(A2, (N + 1) * (M + 1) * (K + 1) + 1, 5);

    for (int i = 0; i < (K + 1); i++)
    {
        for (int j = 0; j < (M + 1); j++)
        {
            for (int l = 0; l < (N + 1); l++)
            {
                A2[por][1] = por;   /// № узла = A2[*][1];
                A2[por][2] = j;     /// Координата x = A2[*][2]
                A2[por][3] = l;     /// Координата у = A2[*][3]
                A2[por][4] = K - i; /// Координата z = A2[*][4]
                por++;
            }
        }
    }
    dim2 = por - 1;

	por = 1;
	setlength<int>(A3, (N + 1) * (M + 1) * (K + 1) + 1, 3);

	/// проверка верхнего слоя бруса, т.е. 1 грани
    if (!ThirdKindCoeff[1][1])  /// задано условие 1 рода на 1 грани
    {
        for (int j = 1; j < M; j++)
        {
            for (int l = 2; l <= N; l++)
            {
                A3[por][1] = j * (N + 1) + l;
                A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], 1);
                por++;
            }
        }
    }
	/// проверяем промежуточные слои
	for (int i = 1; i < K; i++)
	{
		/// по 5 грани бруса
        if (!ThirdKindCoeff[5][1])
        {
			for (int l = 2; l <= N; l++)
			{
				A3[por][1] = i * (M + 1) * (N + 1) + l;
				A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], 5);
				por++;
			}
        }
		/// 4 грань
        if (!ThirdKindCoeff[4][1])
        {
			for (int j = 1; j < M; j++)
			{
				A3[por][1] = i * (M + 1) * (N + 1) + j * (N + 1) + 1;
				A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], 4);
				por++;
			}
        }
		/// 2 грань
        if (!ThirdKindCoeff[2][1])
        {
			for (int j = 1; j < M; j++)
			{
				A3[por][1] = i * (M + 1) * (N + 1) + j * (N + 1) + N + 1;
				A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], 2);
				por++;
			}
        }

		/// 3 грань
        if (!ThirdKindCoeff[3][1])
        {
			for (int l = 2; l <= N; l++)
			{
				A3[por][1] = i * (M + 1) * (N + 1) + M * (N + 1) + l;
				A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], 3);
				por++;
			}
        }

		/// для точек на боковых ребрах (там попадают 2 грани => надо проверять нет ли на 1 из них условия 1 рода)
        if (!ThirdKindCoeff[5][1] || !ThirdKindCoeff[4][1])
		{
			A3[por][1] = i * (M + 1) * (N + 1) + 1;

            int koord = ThirdKindCoeff[5][1] == 0 ? 5 : 4;
			A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], koord);
			por++;
		}
        if (!ThirdKindCoeff[5][1] || !ThirdKindCoeff[2][1]) /// 5+2 грани
		{
			A3[por][1] = i * (M + 1) * (N + 1) + N + 1;

            int koord = ThirdKindCoeff[5][1] == 0 ? 5 : 2;
			A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], koord);
			por++;
		}
        if (!ThirdKindCoeff[2][1] || !ThirdKindCoeff[3][1]) /// 3+2 грани
		{
			A3[por][1] = i * (M + 1) * (N + 1) + (N + 1) * (M + 1);

            int koord = ThirdKindCoeff[2][1] == 0 ? 2 : 3;
			A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], koord);
			por++;
		}
        if (!ThirdKindCoeff[3][1] || !ThirdKindCoeff[4][1]) /// 3+4 грани
		{
			A3[por][1] = i * (M + 1) * (N + 1) + (N + 1) * M + 1;

            int koord = ThirdKindCoeff[3][1] == 0 ? 3 : 4;
			A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], koord);
			por++;
		}
	}
	/// нижний слой - 6 грань
    if (!ThirdKindCoeff[6][1])
    {
        for (int j = 1; j < M; j++)
        {
            for (int l = 2; l <= N; l++)
            {
                A3[por][1] = K * (N + 1) * (M + 1) + j * (N + 1) + l;
                A3[por][2] = bounded(A2[A3[por][1]][2], A2[A3[por][1]][3], A2[A3[por][1]][4], 6);
                por++;
            }
        }
    }

	/// угловые точки, вершины бруса
    corner(1, 2, 5, A2, A3, por, N, M, K);
    corner(1, 4, 5, A2, A3, por, N, M, K);
    corner(1, 2, 3, A2, A3, por, N, M, K);
    corner(1, 3, 4, A2, A3, por, N, M, K);
    corner(6, 2, 5, A2, A3, por, N, M, K);
    corner(6, 4, 5, A2, A3, por, N, M, K);
    corner(6, 2, 3, A2, A3, por, N, M, K);
    corner(6, 3, 4, A2, A3, por, N, M, K);
    xparallelside(1, 2, A2, A3, por, N, M, K);
    xparallelside(1, 4, A2, A3, por, N, M, K);
    xparallelside(6, 2, A2, A3, por, N, M, K);
    xparallelside(6, 4, A2, A3, por, N, M, K);
    yparallelside(1, 3, A2, A3, por, N, M, K);
    yparallelside(1, 5, A2, A3, por, N, M, K);
    yparallelside(6, 3, A2, A3, por, N, M, K);
    yparallelside(6, 5, A2, A3, por, N, M, K);
	dim3 = por - 1;
}
